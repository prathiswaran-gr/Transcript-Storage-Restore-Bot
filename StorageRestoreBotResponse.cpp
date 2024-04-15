
channelUniqueName = "";
// replace with your Channel unique name
botUniqueName = "";
// replace with your Bot unique name
channelID = "";
// replace with your channel ID
ACCOUNT_ID = ;
// replace with your Zoho mail account ID (Type : NUMBER)
TO_DATE = toDate(now.addHour(12).addMinutes(30));
// IST
FROM_DATE = toDate(TO_DATE.subDay(1));
// returns yesterday
SENDER_LIST = {};
// supported from address with multiple DC's
MAIL_LIST = list();
// starting value
transcriptRestoreMap = Map();
transcriptStorageMap = Map();
restoreRowsList = list();
storageRowsList = list();
numberOfRecordsPerRow = "".leftPad(1).toList("");
sortKey = "numberOfOccurences";
transcriptRestoreOccurencesList = list();
transcriptStorageOccurencesList = list();
RESTORE_SUBJECT_TEMPLATE = "ZohoChat - Transcript Restore Error";
STORAGE_SUBJECT_TEMPLATE = "ZohoChat - Transcript Storage Error";
FIELDS = "subject,fromAddress,folderId,messageId";
LIMIT = 5000;
// gets number of mails per call
restoreSerialNumber = 1;
storageSerialNumber = 1;
issueStartValue = 1;
ITERATIONS = "".leftPad(1);
// Number of iterations. At max 1000 mails per response
filteredMails = invokeurl
[
	url :"https://mail.zoho.com/api/accounts/" + ACCOUNT_ID + "/messages/search?searchKey=fromDate:" + FROM_DATE + "::toDate:" + TO_DATE + "::subject:" + STORAGE_SUBJECT_TEMPLATE + "::or:subject:" + RESTORE_SUBJECT_TEMPLATE + "&fields=" + FIELDS + "&start=" + issueStartValue + "&limit=" + LIMIT
	type :GET
	connection:"zmailcliq"
];
for each  mail in filteredMails.get("data")
{
	MAIL_LIST.add(mail);
	// Add fetched mails to the Mail list array
}
for each  mail in MAIL_LIST
{
	fromAddress = mail.get("fromAddress");
	FOLDER_ID = mail.get("folderId");
	MESSAGE_ID = mail.get("messageId");
	if(SENDER_LIST.contains(fromAddress))
	{
		// Validate the from address
		subject = mail.get("subject");
		subject = subject.replaceFirst("-","#");
		subject = subject.replaceFirst("-","#");
		subjectDataList = subject.toList("#");
		transcriptErrorType = "undefined";
		//initial value
		chatId = "undefined";
		//initial value
		if(subjectDataList.size() >= 3)
		{
			transcriptErrorType = subjectDataList.get(1).trim();
			// returns the transcript error type eg. Transcript Storage Error
			chatId = subjectDataList.get(2).trim();
			if(chatId.contains(" - "))
			{
				// it checks whether it contains the string after the chat ID
				endIndex = chatId.lastIndexOf(" - ");
				chatId = chatId.replaceAll(chatId.subString(endIndex,chatId.length()),"");
				// returns the chat ID
			}
		}
		DC = "-";
		// Extracting DC with the help of from address
		if(fromAddress.equals(""))
		{
			DC = "us";
		}
		else if(fromAddress.equals(""))
		{
			DC = "eu";
		}
		else if(fromAddress.equals(""))
		{
			DC = "in";
		}
		else if(fromAddress.equals(""))
		{
			DC = "au";
		}
		chatMapValue = Map();
		chatMapValue.put("dc",DC);
		if(transcriptErrorType.equals("Transcript Restore Error"))
		{
			if(!transcriptRestoreMap.containsKey(chatId))
			{
				chatMapValue.put("numberOfOccurences",1);
			}
			else
			{
				numberOfOccurencesPreviously = toNumber(transcriptRestoreMap.get(chatId).get("numberOfOccurences"));
				chatMapValue.put("numberOfOccurences",numberOfOccurencesPreviously + 1);
			}
			getEmailContent = invokeurl
			[
				url :"https://mail.zoho.com/api/accounts/" + ACCOUNT_ID + "/folders/" + FOLDER_ID + "/messages/" + MESSAGE_ID + "/content"
				type :GET
				connection:"zmailcliq"
			];
			mailContent = getEmailContent.get("data").get("content");
			exceptionStartIndex = mailContent.indexof("Caught");
			exceptionValueStartIndex = 7;
			unformattedException = mailContent.substring(exceptionStartIndex + exceptionValueStartIndex,mailContent.length()).toList("");
			exception = "";
			for each  char in unformattedException
			{
				if(char.equals(" "))
				{
					break;
				}
				exception = exception + char;
			}
			chatMapValue.put("exception",exception);
			transcriptRestoreMap.put(chatId,chatMapValue);
		}
		else if(transcriptErrorType.equals("Transcript Storage Error"))
		{
			if(!transcriptStorageMap.containsKey(chatId))
			{
				chatMapValue.put("numberOfOccurences",1);
			}
			else
			{
				numberOfOccurencesPreviously = toNumber(transcriptStorageMap.get(chatId).get("numberOfOccurences"));
				chatMapValue.put("numberOfOccurences",numberOfOccurencesPreviously + 1);
			}
			getEmailContent = invokeurl
			[
				url :"https://mail.zoho.com/api/accounts/" + ACCOUNT_ID + "/folders/" + FOLDER_ID + "/messages/" + MESSAGE_ID + "/content"
				type :GET
				connection:"zmailcliq"
			];
			mailContent = getEmailContent.get("data").get("content");
			mailContent = htmlDecode(mailContent);
			exceptionStartIndex = mailContent.indexof("Exception");
			exceptionValueStartIndex = 11;
			unformattedException = mailContent.substring(exceptionStartIndex + exceptionValueStartIndex,mailContent.length()).toList("");
			exception = "";
			for each  char in unformattedException
			{
				if(char.equals("<"))
				{
					break;
				}
				exception = exception + char;
			}
			chatMapValue.put("exception",exception);
			transcriptStorageMap.put(chatId,chatMapValue);
		}
	}
}
for each  chatDetails in transcriptRestoreMap
{
	sortKeyValue = chatDetails.get(sortKey);
	transcriptRestoreOccurencesList.add(sortKeyValue);
}
sortedOccurencesList = transcriptRestoreOccurencesList.sort(false);
sortedChatIdBySortedKeyListForRestore = list();
chatIdList = transcriptRestoreMap.keys();
visitedChatIdList = list();
for each  occurrence in sortedOccurencesList
{
	for each  chatDetails in transcriptRestoreMap
	{
		sortKeyValue = chatDetails.get(sortKey);
		if(occurrence == sortKeyValue)
		{
			for each  chatId in chatIdList
			{
				if(visitedChatIdList.contains(chatId))
				{
					continue;
				}
				chatIdDetails = transcriptRestoreMap.get(chatId);
				if(chatIdDetails.get(sortKey) == sortKeyValue)
				{
					sortedChatIdBySortedKeyListForRestore.add(chatId);
					visitedChatIdList.add(chatId);
					break;
				}
			}
		}
	}
}
iterations = toNumber(sortedChatIdBySortedKeyListForRestore.size() / numberOfRecordsPerRow.size());
if(sortedChatIdBySortedKeyListForRestore.size() % numberOfRecordsPerRow.size() != 0)
{
	iterations = iterations + 1;
}
iterations = "".leftPad(iterations).toList("");
chatIndex = 0;
for each  itr in iterations
{
	chatIdText = "";
	dcText = "";
	snoText = "";
	chatIdText = "";
	countText = "";
	exceptionText = "";
	for each  ind in numberOfRecordsPerRow
	{
		chatId = sortedChatIdBySortedKeyListForRestore.get(chatIndex);
		chatDetails = transcriptRestoreMap.get(chatId);
		snoText = snoText + restoreSerialNumber + "\n";
		chatIdText = chatIdText + chatId + "\n";
		dcText = dcText + chatDetails.get("dc") + "\n";
		countText = countText + chatDetails.get("numberOfOccurences") + "\n";
		exceptionText = exceptionText + chatDetails.get("exception") + "\n";
		restoreSerialNumber = restoreSerialNumber + 1;
		chatIndex = chatIndex + 1;
		if(chatIndex >= sortedChatIdBySortedKeyListForRestore.size())
		{
			break;
		}
	}
	snoText = snoText.subString(0,snoText.length() - 1);
	chatIdText = chatIdText.subString(0,chatIdText.length() - 1);
	dcText = dcText.subString(0,dcText.length() - 1);
	countText = countText.subString(0,countText.length() - 1);
	exceptionText = exceptionText.subString(0,exceptionText.length() - 1);
	rowsList0 = Map();
	rowsList0.put("*S.no*",snoText);
	rowsList0.put("Chat ID",chatIdText);
	rowsList0.put("DC",dcText);
	rowsList0.put("Occurrences",countText);
	rowsList0.put("Exception",exceptionText);
	restoreRowsList.add(rowsList0);
}
for each  chatDetails in transcriptStorageMap
{
	sortKeyValue = chatDetails.get(sortKey);
	transcriptStorageOccurencesList.add(sortKeyValue);
}
sortedOccurencesList = transcriptStorageOccurencesList.sort(false);
sortedChatIdBySortedKeyListForStorage = list();
chatIdList = transcriptStorageMap.keys();
visitedChatIdList = list();
for each  occurrence in sortedOccurencesList
{
	for each  chatDetails in transcriptStorageMap
	{
		sortKeyValue = chatDetails.get(sortKey);
		if(occurrence == sortKeyValue)
		{
			for each  chatId in chatIdList
			{
				if(visitedChatIdList.contains(chatId))
				{
					continue;
				}
				chatIdDetails = transcriptStorageMap.get(chatId);
				if(chatIdDetails.get(sortKey) == sortKeyValue)
				{
					sortedChatIdBySortedKeyListForStorage.add(chatId);
					visitedChatIdList.add(chatId);
					break;
				}
			}
		}
	}
}
iterations = toNumber(sortedChatIdBySortedKeyListForStorage.size() / numberOfRecordsPerRow.size());
if(sortedChatIdBySortedKeyListForStorage.size() % numberOfRecordsPerRow.size() != 0)
{
	iterations = iterations + 1;
}
iterations = "".leftPad(iterations).toList("");
chatIndex = 0;
for each  itr in iterations
{
	chatIdText = "";
	dcText = "";
	snoText = "";
	chatIdText = "";
	countText = "";
	exceptionText = "";
	for each  ind in numberOfRecordsPerRow
	{
		chatId = sortedChatIdBySortedKeyListForStorage.get(chatIndex);
		chatDetails = transcriptStorageMap.get(chatId);
		snoText = snoText + storageSerialNumber + "\n";
		chatIdText = chatIdText + chatId + "\n";
		dcText = dcText + chatDetails.get("dc") + "\n";
		countText = countText + chatDetails.get("numberOfOccurences") + "\n";
		exceptionText = exceptionText + chatDetails.get("exception") + "\n";
		storageSerialNumber = storageSerialNumber + 1;
		chatIndex = chatIndex + 1;
		if(chatIndex >= sortedChatIdBySortedKeyListForStorage.size())
		{
			break;
		}
	}
	snoText = snoText.subString(0,snoText.length() - 1);
	chatIdText = chatIdText.subString(0,chatIdText.length() - 1);
	dcText = dcText.subString(0,dcText.length() - 1);
	countText = countText.subString(0,countText.length() - 1);
	exceptionText = exceptionText.subString(0,exceptionText.length() - 1);
	rowsList0 = Map();
	rowsList0.put("*S.no*",snoText);
	rowsList0.put("Chat ID",chatIdText);
	rowsList0.put("DC",dcText);
	rowsList0.put("Occurrences",countText);
	rowsList0.put("Exception",exceptionText);
	storageRowsList.add(rowsList0);
}
restoreSerialNumber = restoreSerialNumber - 1;
storageSerialNumber = storageSerialNumber - 1;
response = Map();
if(storageRowsList.size() == 0)
{
	response.put("text","Transcript stats - " + toString(now,"d/MM/yyyy","Asia/Calcutta") + "\nNumber of restore errors : " + toNumber(restoreSerialNumber));
}
else if(restoreRowsList.size() == 0)
{
	response.put("text","Transcript stats - " + toString(now,"d/MM/yyyy","Asia/Calcutta") + "\nNumber of storage errors : " + toNumber(storageSerialNumber));
}
else
{
	response.put("text","Transcript stats - " + toString(now,"d/MM/yyyy","Asia/Calcutta") + "\nNumber of restore errors : " + toNumber(restoreSerialNumber) + "\nNumber of storage errors : " + toNumber(storageSerialNumber));
}
card = Map();
card.put("theme","prompt");
card.put("title","*Transcript Storage/Restore Errors* ❌");
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
	headersList.add("Occurrences");
	headersList.add("Exception");
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
	headersList.add("Occurrences");
	headersList.add("Exception");
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
