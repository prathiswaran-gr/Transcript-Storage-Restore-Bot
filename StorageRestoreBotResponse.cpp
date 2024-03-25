channelUniqueName = ""; // replace with your Channel unique name
botUniqueName = ""; // replace with your Bot unique name
channelID = ""; // replace with your channel ID
ACCOUNT_ID = ; // replace with your Zoho mail account ID (Type : NUMBER)
TO_DATE = toDate(now.addHour(12).addMinutes(30)); // IST
FROM_DATE = toDate(TO_DATE.subDay(1)); // returns yesterday
SENDER_LIST = {
    "systemgenerated@mailer.zohocliq.com",
    "eu-systemgenerated@eu-mailer.zohochat.com",
    "systemgenerated@mailer.zohochat.in",
    "systemgenerated@mailer.zohochat.com.au"
}; // supported from address with multiple DC's
MAIL_LIST = list();
MAIL_LIST.add(-1); // starting value
RESTORE_SUBJECT_TEMPLATE = "ZohoChat - Transcript Restore Error";
STORAGE_SUBJECT_TEMPLATE = "ZohoChat - Transcript Storage Error";
FIELDS = "subject,fromAddress,folderId,messageId";
LIMIT = 100; // gets number of mails per call
transcriptRestoreDomainMap = Map();
transcriptStorageDomainMap = Map();
restoreSerialNumber = 1;
storageSerialNumber = 1;
restoreRowsList = list();
storageRowsList = list();
transcriptRestoreDomainMap.put("com", Map());
transcriptRestoreDomainMap.put("eu", Map());
transcriptRestoreDomainMap.put("in", Map());
transcriptRestoreDomainMap.put("au", Map());
transcriptStorageDomainMap.put("com", Map());
transcriptStorageDomainMap.put("eu", Map());
transcriptStorageDomainMap.put("in", Map());
transcriptStorageDomainMap.put("au", Map());
issueStartValue = 1;
for each mailDetials in MAIL_LIST {
    filteredMails = invokeurl[
        url: "https://mail.zoho.com/api/accounts/" + ACCOUNT_ID + "/messages/search?searchKey=fromDate:" + FROM_DATE + "::toDate:" + TO_DATE + "::subject:" + STORAGE_SUBJECT_TEMPLATE + "::or:subject:" + RESTORE_SUBJECT_TEMPLATE + "&fields=" + FIELDS + "&start=" + issueStartValue + "&limit=" + LIMIT type: GET connection: "zmailcliq"
    ];
    for each mail in filteredMails.get("data") {
        MAIL_LIST.add(mail); // Add fetched mails to the Mail list array
    }
    if (filteredMails.get("data").length() < LIMIT) {
        // break the loop if the number of mails is less than the limit value
        break;
    } else {
        issueStartValue = issueStartValue + LIMIT; // increase the start value for the further API calls
    }
}
for each mail in MAIL_LIST {
    if (mail == -1) {
        continue;
    }
    fromAddress = mail.get("fromAddress");
    FOLDER_ID = mail.get("folderId");
    MESSAGE_ID = mail.get("messageId");
    if (SENDER_LIST.contains(fromAddress)) { // Validate the from address
        subject = mail.get("subject");
        subject = subject.replaceFirst("-", "#");
        subject = subject.replaceFirst("-", "#");
        subjectDataList = subject.toList("#");
        transcriptErrorType = "undefined"; //initial value
        chatId = "undefined";  //initial value
        if (subjectDataList.size() >= 3) {
            transcriptErrorType = subjectDataList.get(1).trim(); // returns the transcript error type eg. Transcript Storage Error
            chatId = subjectDataList.get(2).trim();
            if (chatId.contains(" - ")) { // it checks whether it contains the string after the chat ID
                endIndex = chatId.lastIndexOf(" - ");
                chatId = chatId.replaceAll(chatId.subString(endIndex, chatId.length()), "");
                // returns the chat ID
            }
        }
        DC = "-";
        // Extracting DC with the help of from address
        if (fromAddress.equals("systemgenerated@mailer.zohocliq.com")) {
            DC = "com";
        } else if (fromAddress.equals("eu-systemgenerated@eu-mailer.zohochat.com")) {
            DC = "eu";
        } else if (fromAddress.equals("systemgenerated@mailer.zohochat.in")) {
            DC = "in";
        } else if (fromAddress.equals("systemgenerated@mailer.zohochat.com.au")) {
            DC = "au";
        }
        chatMapValue = Map();
        chatDetailsMap = Map();
        if (transcriptErrorType.equals("Transcript Restore Error")) {
            chatDetailsMap = transcriptRestoreDomainMap.get(DC);
            if (!chatDetailsMap.containsKey(chatId)) {
                chatMapValue.put("numberOfOccurences", 1);
            } else {
                numberOfOccurencesPreviously = toNumber(chatDetailsMap.get(chatId).get("numberOfOccurences"));
                chatMapValue.put("numberOfOccurences", numberOfOccurencesPreviously + 1);
            }
            getEmailContent = invokeurl[
                url: "https://mail.zoho.com/api/accounts/" + ACCOUNT_ID + "/folders/" + FOLDER_ID + "/messages/" + MESSAGE_ID + "/content"
                type: GET connection: "zmailcliq"
            ];
            mailContent = getEmailContent.get("data").get("content");
            exceptionStartIndex = mailContent.indexof("Caught");
            exceptionValueStartIndex = 7;
            unformattedException = mailContent.substring(exceptionStartIndex + exceptionValueStartIndex, mailContent.length()).toList("");
            exception = "";
            for each char in unformattedException {
                if (char.equals(" ")) {
                    break;
                }
                exception = exception + char;
            }
            chatMapValue.put("Exception", exception);
            chatDetailsMap.put(chatId, chatMapValue);
        } else if (transcriptErrorType.equals("Transcript Storage Error")) {
            chatDetailsMap = transcriptStorageDomainMap.get(DC);
            if (!chatDetailsMap.containsKey(chatId)) {
                chatMapValue.put("numberOfOccurences", 1);
            } else {
                numberOfOccurencesPreviously = toNumber(chatDetailsMap.get(chatId).get("numberOfOccurences"));
                chatMapValue.put("numberOfOccurences", numberOfOccurencesPreviously + 1);
            }
            getEmailContent = invokeurl[
                url: "https://mail.zoho.com/api/accounts/" + ACCOUNT_ID + "/folders/" + FOLDER_ID + "/messages/" + MESSAGE_ID + "/content"
                type: GET connection: "zmailcliq"
            ];
            mailContent = getEmailContent.get("data").get("content");
            mailContent = htmlDecode(mailContent);
            exceptionStartIndex = mailContent.indexof("Exception");
            exceptionValueStartIndex = 11;
            unformattedException = mailContent.substring(exceptionStartIndex + exceptionValueStartIndex, mailContent.length()).toList("");
            exception = "";
            for each char in unformattedException {
                if (char.equals("<")) {
                    break;
                }
                exception = exception + char;
            }
            chatMapValue.put("exception", exception);
            chatDetailsMap.put(chatId, chatMapValue);
        }
    }
}
domainList = transcriptRestoreDomainMap.keys();
domainIndex = 0;
for each domain in transcriptRestoreDomainMap {
    chatList = domain.keys();
    chatIndex = 0;
    for each chatDetails in domain {
        chatId = chatList.get(chatIndex);
        dc = domainList.get(domainIndex);
        rowsList0 = Map();
        rowsList0.put("*S.no*", restoreSerialNumber);
        rowsList0.put("Chat ID", chatId);
        rowsList0.put("DC", dc);
        rowsList0.put("Unique Count", chatDetails.get("numberOfOccurences"));
        rowsList0.put("Exception", chatDetails.get("exception"));
        restoreRowsList.add(rowsList0);
        restoreSerialNumber = restoreSerialNumber + 1;
        chatIndex = chatIndex + 1;
    }
    domainIndex = domainIndex + 1;
}
domainIndex = 0;
for each domain in transcriptStorageDomainMap {
    chatList = domain.keys();
    chatIndex = 0;
    for each chatDetails in domain {
        chatId = chatList.get(chatIndex);
        dc = domainList.get(domainIndex);
        rowsList0 = Map();
        rowsList0.put("*S.no*", storageSerialNumber);
        rowsList0.put("Chat ID", chatId);
        rowsList0.put("DC", dc);
        rowsList0.put("Unique Count", chatDetails.get("numberOfOccurences"));
        rowsList0.put("Exception", chatDetails.get("exception"));
        storageRowsList.add(rowsList0);
        storageSerialNumber = storageSerialNumber + 1;
        chatIndex = chatIndex + 1;
    }
    domainIndex = domainIndex + 1;
}
response = Map();
if (storageRowsList.size() == 0) {
    response.put("text", "Transcript stats - " + toString(now, "d/MM/yyyy", "Asia/Calcutta") + "\nNumber of restore errors : " + restoreRowsList.size());
} else if (restoreRowsList.size() == 0) {
    response.put("text", "Transcript stats - " + toString(now, "d/MM/yyyy", "Asia/Calcutta") + "\nNumber of storage errors : " + storageRowsList.size());
} else {
    response.put("text", "Transcript stats - " + toString(now, "d/MM/yyyy", "Asia/Calcutta") + "\nNumber of storage errors : " + storageRowsList.size() + "\nNumber of restore errors : " + restoreRowsList.size());
}

card = Map();
card.put("theme", "prompt");
card.put("title", "*Transcript Storage/Restore Errors* ❌");
response.put("card", card);
slidesList = list(); // global slideslist
if (restoreRowsList.size() != 0) {
    slidesList0 = Map();
    slidesList0.put("type", "table");
    slidesList0.put("title", "Transcript Restore Error 🐞");
    data = Map();
    headersList = list();
    headersList.add("*S.no*");
    headersList.add("Chat ID");
    headersList.add("DC");
    headersList.add("Unique Count");
    headersList.add("Exception");
    data.put("headers", headersList);
    data.put("rows", restoreRowsList); // add all rows related to restore error
    slidesList0.put("data", data);
    slidesList.add(slidesList0);
}
if (storageRowsList.size() != 0) {
    slidesList1 = Map();
    slidesList1.put("type", "table");
    slidesList1.put("title", "Transcript Storage Error 🐞");
    data = Map();
    headersList = list();
    headersList.add("*S.no*");
    headersList.add("Chat ID");
    headersList.add("DC");
    headersList.add("Unique Count");
    headersList.add("Exception");
    data.put("headers", headersList);
    data.put("rows", storageRowsList); // add all rows related to storage error
    slidesList1.put("data", data);
    slidesList.add(slidesList1);
}
response.put("slides", slidesList);
info response;
if (storageRowsList.size() != 0 || restoreRowsList.size() != 0) {
    zoho.cliq.postToChannelAsBot(channelUniqueName, botUniqueName, response);
}
