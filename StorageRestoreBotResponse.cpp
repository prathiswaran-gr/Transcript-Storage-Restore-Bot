channelUniqueName = ""; // replace with your Channel unique name
botUniqueName = ""; // replace with your Bot unique name
channelID = ""; // replace with your channel ID
ACCOUNT_ID = ""; // replace with your Zoho mail account ID
TO_DATE = toDate(now.addHour(12).addMinutes(30)); // IST
FROM_DATE = toDate(TO_DATE.subDay(1)); // returns yesterday
SENDER_LIST = {"systemgenerated@mailer.zohocliq.com","eu-systemgenerated@eu-mailer.zohochat.com","systemgenerated@mailer.zohochat.in","systemgenerated@mailer.zohochat.com.au"}; // supported from address with multiple DC's
MAIL_LIST = list();
MAIL_LIST.add(-1); // starting value
RESTORE_SUBJECT_TEMPLATE = "ZohoChat - Transcript Restore Error";
STORAGE_SUBJECT_TEMPLATE = "ZohoChat - Transcript Storage Error";
FIELDS = "subject,fromAddress";
LIMIT = 100; // maximum value
transcriptRestoreMap = Map();
transcriptStorageMap = Map();
restoreSerialNumber = 1;
storageSerialNumber = 1;
restoreRowsList = list();
storageRowsList = list();
issueStartValue = 1;
for each  mailDetials in MAIL_LIST
{
	filteredMails = invokeurl
	[
		url :"https://mail.zoho.in/api/accounts/" + ACCOUNT_ID + "/messages/search?searchKey=fromDate:" + FROM_DATE + "::toDate:" + TO_DATE + "::subject:" + STORAGE_SUBJECT_TEMPLATE + "::or:subject:" + RESTORE_SUBJECT_TEMPLATE + "&fields=" + FIELDS + "&start=" + issueStartValue+"&limit="+LIMIT
		type :GET
		connection:"oauth"
	];
	for each  mail in filteredMails.get("data")
	{
		MAIL_LIST.add(mail); // Add fetched mails to the Mail list array
	}
	if(filteredMails.get("data").length() < LIMIT)
	{
		// break the loop if the number of mails is less than the limit value
		break;
	}
	else
	{
		issueStartValue = issueStartValue + LIMIT; // increase the start value for the further API calls
	}
}
for each mail in MAIL_LIST
{
	if(mail == -1){
		continue;
	}
	fromAddress = mail.get("fromAddress");
	if(SENDER_LIST.contains(fromAddress)) // Validate the from address
	{
		
		subject = mail.get("subject");
		
		subject = subject.replaceFirst("-", "#");
		subject = subject.replaceFirst("-", "#");
		subjectDataList = subject.toList("#");
		transcriptErrorType = "undefined"; //initial value
		chatId = "undefined"; //initial value
		if(subjectDataList.size() == 3)
		{
			transcriptErrorType = subjectDataList.get(1).trim();
			// returns the transcript error type eg. Transcript Storage Error
			chatId = subjectDataList.get(2).trim();
			// returns the chat ID
		}
		DC = "-";
		// Extracting DC with the help of from address
		if(fromAddress.equals("systemgenerated@mailer.zohocliq.com"))
		{
			DC = "com";
		}
		else if(fromAddress.equals("eu-systemgenerated@eu-mailer.zohochat.com"))
		{
			DC = "eu";
		}
		else if(fromAddress.equals("systemgenerated@mailer.zohochat.in"))
		{
			DC = "in";
		}
		else if(fromAddress.equals("systemgenerated@mailer.zohochat.com.au"))
		{
			DC = "au";
		}
		mapValue = Map();
		mapValue.put("DC",DC);
		if(transcriptErrorType.equals("Transcript Restore Error"))
		{
			if(!transcriptRestoreMap.containsKey(chatId))
			{
				mapValue.put("numberOfOccurences",1);
			}
			else
			{
				numberOfOccurencesPreviously = transcriptRestoreMap.get(chatId).get("numberOfOccurences");
				mapValue.put("numberOfOccurences",numberOfOccurencesPreviously + 1);
			}
			transcriptRestoreMap.put(chatId,mapValue);
			rowsList0 = Map();
			rowsList0.put("*S.no*",restoreSerialNumber);
			rowsList0.put("Chat ID",chatId);
			rowsList0.put("DC",transcriptRestoreMap.get(chatId).get("DC"));
			rowsList0.put("Unique Chats",transcriptRestoreMap.get(chatId).get("numberOfOccurences"));
			restoreRowsList.add(rowsList0);
			restoreSerialNumber = restoreSerialNumber + 1;
		}
		else if(transcriptErrorType.equals("Transcript Storage Error"))
		{
			if(!transcriptStorageMap.containsKey(chatId))
			{
				mapValue.put("numberOfOccurences",1);
			}
			else
			{
				numberOfOccurencesPreviously = transcriptStorageMap.get(chatId).get("numberOfOccurences");
				mapValue.put("numberOfOccurences",numberOfOccurencesPreviously + 1);
			}
			transcriptStorageMap.put(chatId,mapValue);
			rowsList0 = Map();
			rowsList0.put("*S.no*",storageSerialNumber);
			rowsList0.put("Chat ID",chatId);
			rowsList0.put("DC",transcriptStorageMap.get(chatId).get("DC"));
			rowsList0.put("Unique Chats",transcriptStorageMap.get(chatId).get("numberOfOccurences"));
			storageRowsList.add(rowsList0);
			storageSerialNumber = storageSerialNumber + 1;
		}
	}
}
response = Map();
response.put("text","*Zoho Chat* 💬");
card = Map();
card.put("icon","https://cdn-icons-png.freepik.com/256/3558/3558860.png");
card.put("theme","modern-inline");
response.put("card",card);
slidesList = list();
// global slideslist
if(restoreRowsList.size() != 0)
{
	slidesList0 = Map();
	slidesList0.put("type","table");
	slidesList0.put("title","Transcript Restore Error 🐞");
	data = Map();
	headersList = list();
	headersList.add("*S.no*");
	headersList.add("Chat ID");
	headersList.add("DC");
	headersList.add("Unique Chats");
	data.put("headers",headersList);
	data.put("rows",restoreRowsList);
	// add all rows related to restore error
	slidesList0.put("data",data);
	slidesList.add(slidesList0);
}
if(storageRowsList.size() != 0)
{
	slidesList1 = Map();
	slidesList1.put("type","table");
	slidesList1.put("title","Transcript Storage Error 🐞");
	data = Map();
	headersList = list();
	headersList.add("*S.no*");
	headersList.add("Chat ID");
	headersList.add("DC");
	headersList.add("Unique Chats");
	data.put("headers",headersList);
	data.put("rows",storageRowsList);
	// add all rows related to storage error
	slidesList1.put("data",data);
	slidesList.add(slidesList1);
}
response.put("slides",slidesList);
if(storageRowsList.size() != 0 || restoreRowsList.size() != 0)
{
	zoho.cliq.postToChannelAsBot(channelUniqueName,botUniqueName,response);
}

