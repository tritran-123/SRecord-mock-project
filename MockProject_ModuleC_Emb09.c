/***********************************************************************************/
/*                                EMB09-TRAN MINH TRI                              */
/*                          MOCK PROJECT S_REC APPLICATION                         */
/*               THANK YOU TO MR NGUYEN FOR BRINGING A VERY USEFUL COURSE C        */
/*                                                                                 */
/***********************************************************************************/

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#define RECORDSTART 'S' /* define const recordstart always = 's',if there have any change,will be changed charater here*/

/* General description of code organization :
    +Part 1 : Write functions that purpose is to calculate and determine the value for each field in the struct in SREC file format
    +Part 2 : Write functions to determine the format of a segment in the SREC file include : FirstLine (S0) - DataLine(S1||S2||S3) - LastLine(S9||S8||S7)
    +Part 3 : Write functions to response the programing requirement (Feature 1-2-3-4)
    +Part 4 : Write functions to include Menu and User Manual to help user can use conveniently.
*/

/* Part 1 */
/* Declare struct according to format S-Record in Wiki */
struct tFormatSRecord
{
  char cRecordStart;
  char cType;         /* single numeric digit "0" to "9" character */
  char sByteCount[3]; /* two hex digits ("00" to "FF") = byte in (address + data + checksum) */
  char sAddress[9];   /*  four / six / eight hex digits as determined by the record type */
  char sData[1024];   /* a sequence of 2n hex digits, for n bytes of the data*/
  char sCheckSum[3];  /*  two hex digits */
};
typedef struct tFormatSRecord tFormatSRecord;

/* This purpose of this parseString func is return 1 value name "record" type struct
 and this record already parse each component field in struct*/
tFormatSRecord parseString(char *str) /* transmit 1 string need to parse in parameter */
{
  int len_str = strlen(str);    /* determine lenght string */
  if (str[len_str - 1] == '\n') /* file Srec will receive '\n' in each line so need to clear this in each string*/
  {
    len_str -= 1;
  }
  tFormatSRecord record;             /* Declare 1 variable type struct to conduct parse each component */
  record.cRecordStart = RECORDSTART; /* recordstart auto = 'S' */

  record.cType = str[1]; /* cType located in position [1] in string */

  strncpy(record.sByteCount, str + 2, sizeof(char) * 2); /* bytecount = copy 2 charater in position[2] and[3] in string*/
  record.sByteCount[2] = '\0';

  int address_len = 4; /* lenght regulation in address = 4/6/8 digit dependent 16/24/32bit */
  if (record.cType == '2')
    address_len = 6;
  if (record.cType == '3')
    address_len = 8;

  strncpy(record.sAddress, str + 4, sizeof(char) * address_len); /* sAddress = copy number of character after field
                                                                    sBytecount(position start copy = str +4) */

  record.sAddress[address_len] = '\0';

  int data_len = len_str - 1 - 1 - 2 - address_len - 2; /* lenght data = lenght string - number charater (remaining component)*/
  if (data_len > 0)
  {
    record.sData[data_len] = '\0';
    strncpy(record.sData, str + 4 + address_len, sizeof(char) * data_len);
  }

  strncpy(record.sCheckSum, str + 4 + address_len + data_len, sizeof(char) * 2); /* copy checksum = 2digit in hex = 1byte */
  record.sCheckSum[2] = '\0';

  return record; /* return value */
}

/* File Srec will organization in mode address ascending(0->FFFFFFFF) in file so need to identify last address(max address currently)
to continues append */
/* purpose of this func : read file srec and use value return in func(parseString) to read value each component and return
highest address in end file*/
long HighestAddressInFile(char filename[]) /* parameter = filename want to read and determine return address */
{
  long iRetval = 0; /* Declare variable to receive highest address */
  FILE *fptr = fopen(filename, "r");

  char line[1024]; /* Declare this varialbe to read each line in file */

  while (fgets(line, 1024, fptr))
  {

    if (strlen(line) > 0)
    {
      tFormatSRecord record = parseString(line); /* Declare variable to parse each string */

      if (record.cType != '9' && record.cType != '8' && record.cType != '7') /* No need to determine address in S0||7||8||9 */
      {
        long iAddrCeil = strtol(record.sAddress, NULL, 16) + strlen(record.sData) / 2; /* use strtol to convert value in type string to type long in base hex*/
        iRetval = iAddrCeil; /* assign last address in file srec = addres + (byte data=lenght data/2 (2digit = 1byte)) to retval */
      }
    }
  }
  fclose(fptr);
  return iRetval;
}

/* func calculate checksum in receip : 0xFF - (sum & 0xFF) */
long CalCulateChecksum(tFormatSRecord record)
{
  long iSum = 0x00;
  char sString[3];
  sString[2] = '\0';
  iSum += strtol(record.sByteCount, NULL, 16); /* start add value in bytecount (use strtol to convert string to value long )*/

  for (int i = 0; i < strlen(record.sAddress) / 2; i++)
  {
    strncpy(sString, record.sAddress + i * 2, sizeof(char) * 2); /* copy sequence 2digit in address into sString */
    iSum += strtol(sString, NULL, 16);                           /* add value in 2digit address in type long */
  }
  for (int i = 0; i < strlen(record.sData) / 2; i++) /* similar address->continues add value data in 2digit type long */
  {
    strncpy(sString, record.sData + i * 2, sizeof(char) * 2);
    iSum += strtol(sString, NULL, 16);
  }

  return 0xFF - (iSum & 0xFF); /* return value in receip cal checksum */
}

/* Use above functions to declare and format the lines in
the srec file including: firstline(S0), dataline(S1||S2||S3), lastline(S9||S8||S7) */

/*Part 2*/
/* Func declare and determine value in each time srec format start :  FirstLine(S0) */
tFormatSRecord SRecFirstLineInFormat(char sDataInS0NeedToConvert[]) /* param =  data string S0 want to write in file */
{
  /* purpose line 125-137 : Convert sequence character in S0 from type String to char(ASci) in base 16 */
  char sHeaderS0[100];
  strcpy(sHeaderS0, sDataInS0NeedToConvert); /* copy data S0 to sHeader (data string should < 100 character)*/
  char sHeaderS0InHex[200];                  /* Declare sheaderS0InHex to save value data S0 when convert each charater to char(Asci-base 16-1byte char need 2element(2digit in Hex)) */
  char sConvertCharacterToHex[3];            /* Declare sConvertCharacterToHex = save each character in the header in hex format (%02X) */
  sConvertCharacterToHex[2] = '\0';
  size_t len = strlen(sHeaderS0);
  for (int i = 0; i < len; i++) /* Loop go through each character in string and convert it to format %02x base in Asci*/
  {
    sprintf(sConvertCharacterToHex, "%02x", sHeaderS0[i]);
    sHeaderS0InHex[2 * i] = sConvertCharacterToHex[0];
    sHeaderS0InHex[2 * i + 1] = sConvertCharacterToHex[1];
  }
  sHeaderS0InHex[len * 2] = '\0'; /* Determine the end of the string*/

  tFormatSRecord FirstLine; /* Declare variable comply with format Header */
  FirstLine.cRecordStart = RECORDSTART;
  FirstLine.cType = '0'; /* "S0" */

  int iCountFullByteInS0 = 0;
  iCountFullByteInS0 = 2 + strlen(sDataInS0NeedToConvert) + 1; /* +1 byte check summ and +2 byte address 0000 */
  sprintf(FirstLine.sByteCount, "%02X", iCountFullByteInS0);   /* print value into Field sByteCount in format Hex */

  sprintf(FirstLine.sAddress, "%04X", 0); /* print value into Field sByteCount in format Hex */

  strcpy(FirstLine.sData, sHeaderS0InHex); /* copy value data into Field sData in format Hex */

  sprintf(FirstLine.sCheckSum, "%02X", CalCulateChecksum(FirstLine)); /* print value checksum already be calculated into Field
                                                                      sChecksum in format Hex */
  return FirstLine;
}

/* Before writing the function  Dataline and LastLine, realize that cType and sAddress are related to
the address arrangement structure of the specified program: for example, S1 stores data with 16bits, S2 stores data with 24bits...
->So to avoid (if else) multiple times in the code -> write a function that determines the ctype base in last address*/

/* Purpose func GetAddressType() : Determine field ctype of dataline and lastline */
char GetAddressType(long iLastAddress, int iflag, char *sAddress) /*Parameter = value last address in file srec to determin ctype,
iflag to determine value func() return When to use it for dataline,when to use it for lastline, Pointer sAddress to save value
(The first address value of the data each time cType is converted) */
{
  if (iLastAddress <= 0xFFFF) /* Condition for cType '1' or '9' (address just in 16bit 0000->ffff) */
  {
    if (sAddress != NULL)
    {
      sprintf(sAddress, "%04X", 0); /* Start Address when cType dataline = '1' */
    }
    if (iflag == 0) /* use value return to cType's LastLine when iflag == 0 */
      return '9';
    else /* use value return to cType's DataLine when iflag != 0 */
      return '1';
  }
  else if (iLastAddress > 0xFFFF && iLastAddress <= 0xFFFFFF) /* Condition for cType '2' or '8' (address just in 24bit 00ffff->ffffff) */
  {
    if (sAddress != NULL)
    {
      sprintf(sAddress, "%06X", 0x01FFFF); /* Start Address when cType dataline = '2' */
    }
    if (iflag == 0) /* use flag same above */
      return '8';
    else
      return '2';
  }
  else if (iLastAddress > 0xFFFFFF && iLastAddress <= 0xFFFFFFFF) /* Condition for cType '3' or '7' (address just in 32bit 00ffffff->ffffffff) */
  {
    if (sAddress != NULL)
    {
      sprintf(sAddress, "%08X", 0x01FFFFFF); /* Start Address when cType dataline = '3' */
    }
    if (iflag == 0) /* use flag same above */
      return '7';
    else
      return '3';
  }
  return '0';
}

/* Func declare and determine value in each time format srec end : Lastline (S7||S8||S9) */
tFormatSRecord SRecLastLineInformat()
{
  tFormatSRecord LastLine;
  LastLine.cRecordStart = RECORDSTART;

  LastLine.cType = GetAddressType(HighestAddressInFile("S_Record.srec"), 0, LastLine.sAddress); /* iflag = 0 return value in lastline(9 or 8 or 7) */
                                                                                                /* Value field sAddress's LastLine also determine in func(GetAddressType)*/
  sprintf(LastLine.sByteCount, "%02X", strlen(LastLine.sAddress) + 1);                          /* Printf value into sByeCount in format Hex*/

  sprintf(LastLine.sCheckSum, "%02X", CalCulateChecksum(LastLine)); /* print value checksum already be calculated into Field sChecksum in format Hex */

  return LastLine;
}

/* Func declare and determine value in each time format srec save data : DataLine (S1||S2||S3) */
/* With field sData in Dataline have 2 case : In Feature 1(string sValue) is value user input from keyboard (completely int) so I
can turn it from type string to type value by atoi() and printf it into field sData but in Feature 2 and 3 (string sValue) is data read
from binary file, not int data so need 1 variable (iflag) to determine When do need to return the sData value for feature 1?
and return the sData value for feature 2and3?*/
tFormatSRecord SRecDataLineInFormat(char sValue[], long iAddressToWrite, int iflag)
{
  tFormatSRecord DataLine;
  DataLine.cRecordStart = RECORDSTART; /* Asign value field */

  DataLine.cType = GetAddressType(iAddressToWrite, 1, NULL); /* Asign value field cType (use iflag = 1 to return value cType in
                                                           dataline(1 or 2 or 3)),Don't use last variable->Assign pointer = Null*/

  sprintf(DataLine.sData, "%X", atoi(sValue)); /* atoi to return value in string back to int to print this into field in format Hex */

  if (iflag == 1) /* if flag == 1 it sData use for Feature 2 and 3 (sValue is data read from binary file) so  Use the same
                  conversion method as when converting data of S0*/
  {
    char sValueInHex[400];
    char sConvertCharacterToHex[3];
    sConvertCharacterToHex[2] = '\0';
    size_t len = strlen(sValue);
    for (int i = 0; i < len; i++)
    {
      sprintf(sConvertCharacterToHex, "%02x", sValue[i]);
      sValueInHex[2 * i] = sConvertCharacterToHex[0];
      sValueInHex[2 * i + 1] = sConvertCharacterToHex[1];
    }
    sValueInHex[len * 2] = '\0';
    strcpy(DataLine.sData, sValueInHex);
  }
  /* The number of digits in the data must be even number */
  if (strlen(DataLine.sData) % 2 != 0)
  {
    for (int i = strlen(DataLine.sData); i >= 0; i--)
    {
      DataLine.sData[i + 1] = DataLine.sData[i];
    }
    DataLine.sData[0] = '0';
  }
  /* sByteCount = Byte address + Byte data + byte checksum and print into sByteCount in format Hex*/
  sprintf(DataLine.sByteCount, "%02X", ((int)(DataLine.cType) - 47) + strlen(DataLine.sData) / 2 + 1); /*press ctype('1' or '2' or '3')
    from char to int and - 47 will have value byte address (Ex : cType = '1'->(int)('1') - 47 = 2 = byte address respectively)*/

  long iCurrentAddress = iAddressToWrite; /* The format in Hex of sAddress is different for each cType*/
  if (DataLine.cType == '1')
  {
    sprintf(DataLine.sAddress, "%04X", iCurrentAddress);
  }
  else if (DataLine.cType == '2')
  {
    sprintf(DataLine.sAddress, "%06X", iCurrentAddress);
  }
  else if (DataLine.cType == '3')
  {
    sprintf(DataLine.sAddress, "%08X", iCurrentAddress);
  }

  sprintf(DataLine.sCheckSum, "%02X", CalCulateChecksum(DataLine)); /* print value checksum already be calculated into Field sChecksum in format Hex */
  return DataLine;
}

/* Function's purpose : Convert value in type struct to type string */
void SRecStructToString(char *str_representation, tFormatSRecord record)
{
  sprintf(str_representation, "%c%c%s%s%s%s", record.cRecordStart, record.cType,
          record.sByteCount, record.sAddress, record.sData, record.sCheckSum);
}

/* Part 3 */
/*------------------------------------------------------(Feature 1)------------------------------------------------------------*/

/* Function call user input sequence int value and convert to Record format and write in file .srec */
void InputValueAndWriteInFileSRec()
{
  char sValueInput[1024];
  printf("Input value of elements (maximum 100 number type int in 1 entry) and comply with format already notify in part User Manual: ");
  gets(sValueInput);
  for (int i = 0; i < 1024; i++) /* Check end string when have '\n'*/
  {
    if (sValueInput[i] == '\n')
      sValueInput[i] = '\0';
    break;
  }
  /* append mode: take previous highest, write mode: just assign 0*/
  long iCurrentAddress = HighestAddressInFile("S_Record.srec");

  /* open file in mode append */
  FILE *fRecord = fopen("S_Record.srec", "a");
  if (fRecord == NULL)
  {
    printf("\nError file open !");
    return;
  }

  char line[1024]; /* Declare 1 string to save value when convert from type struct to type string */

  SRecStructToString(line, SRecFirstLineInFormat("Convert Int Value To SRec")); /*Receive value lastline(Record format) in type struct and convert to string to write in file .srec */
  fprintf(fRecord, "%s\n", line);

  char *token = strtok(sValueInput, ","); /* use strtok to separate each number when user input sequence and seperate by commos*/
  tFormatSRecord Record;                  /* Declare this variable to calculate value of sAddress after + Byte data to append new dataline*/
  strcpy(Record.sData, "");
  while (token != NULL) /* Condition to receive number in type string */
  {
    iCurrentAddress += strlen(Record.sData) / 2;
    Record = SRecDataLineInFormat(token, iCurrentAddress, 0); /* Call SRecDataLineInFormat(#feature 1 so flag = 0) to receive value already in format Record*/
    SRecStructToString(line, Record);
    fprintf(fRecord, "%s\n", line);
    token = strtok(NULL, " , ");
  }
  SRecStructToString(line, SRecLastLineInformat()); /*Receive value lastline(Record format) in type struct and convert to string to write in file .srec */
  fprintf(fRecord, "%s\n", line);
  printf("\nAlready convert and append data.Access file: S_Record.srec to see SREC format !");
  fclose(fRecord); /* Close file*/
}

/*-----------------------------------------------------(Feature 2 and 3)----------------------------------------------------------*/

/* Use flag to distinguish between feature 2 and 3 (flag == 1 is mode write Feature 3 and != 1 is mode append in old Srec file feature 2)*/
void InputFileBinaryPath(int iflag)
{
  char FileBinaryPath[256];
  printf("\nInput file binary path need to convert and comply with format already notify in part User Manual: ");
  gets(FileBinaryPath);
  for (int i = 0; i < 256; i++)
  {
    if (FileBinaryPath[i] == '\n') /* Check end string when have '\n'*/
      FileBinaryPath[i] = '\0';
    break;
  }

  FILE *fBinary = fopen(FileBinaryPath, "rb"); /* Open file binary in above path in mode rb */
  if (fBinary == NULL)
  {
    printf("\nError binary file open !");
    return;
  }

  long iCurrentAddress = HighestAddressInFile("S_Record.srec"); /* Similar Feature 1,determine address continues to append */
  FILE *fRecord = fopen("S_Record.srec", "a");                  /* open file record */
  if (fRecord == NULL)
  {
    printf("\nError file open !");
    return;
  }
  if (iflag == 1) /* if flag == 1 this is use for Feature 3 */
  {
    long iCurrentAddress = 0;                        /* Declare new address start at 0 */
    fclose(fRecord);                                 /* close file record already open in mode 'a' and open again in mode 'w' write in new file */
    FILE *fRecord = fopen("New_S_Record.srec", "w"); /* open New file path in mode 'w' */
    if (fRecord == NULL)
    {
      printf("\nError file open !");
      return;
    }
  }

  char sByteRead[200];                                                            /* declare 1 string lenght 200 to save 200 byte be read in file binary */
  char line[1024];                                                                /* similar Feature 1*/
  SRecStructToString(line, SRecFirstLineInFormat("Convert Binary File To SRec")); /* FirstLine */
  fprintf(fRecord, "%s\n", line);

  tFormatSRecord Record;
  strcpy(Record.sData, "");
  while (fread(sByteRead, 1, sizeof(sByteRead), fBinary) > 0) /* loop to read data from binary and convert to Record format and write in file */
  {
    iCurrentAddress += strlen(Record.sData) / 2;
    Record = SRecDataLineInFormat(sByteRead, iCurrentAddress, 1); /* similar Feature 1*/
    SRecStructToString(line, Record);
    fprintf(fRecord, "%s\n", line);
  }

  SRecStructToString(line, SRecLastLineInformat()); /* LastLine */
  fprintf(fRecord, "%s\n", line);

  printf("\nAlready read and convert data.Access file to see SREC format !");
  fclose(fBinary);
  fclose(fRecord);
}

/*----------------------------------------------------------feature 3 --------------------------------------------------------*/
void CheckForCorruption()
{
  char FileSrecPathNeedToCheck[256];
  printf("\nInput file binary path need to convert and comply with format already notify in part User Manual: ");
  gets(FileSrecPathNeedToCheck);
  for (int i = 0; i < 256; i++)
  {
    if (FileSrecPathNeedToCheck[i] == '\n') /* Check end string when have '\n'*/
      FileSrecPathNeedToCheck[i] = '\0';
    break;
  }
  FILE *fRecord = fopen(FileSrecPathNeedToCheck, "r"); /* open file need to check in mode read */
  if (fRecord == NULL)
  {
    printf("\nError file open !");
    return;
  }

  char line[512]; /* Declare string to save each line when read data */
  while (fgets(line, 512, fRecord))
  {
    tFormatSRecord record = parseString(line);                /* variable record in type struct and already parse each field */
    long ComputedChecksum = CalCulateChecksum(record);        /* calculate checksum */
    long ReadedChecksum = strtol(record.sCheckSum, NULL, 16); /* return value checksum in line in file need check to type long to compare*/
    if (ComputedChecksum != ReadedChecksum)
    {
      printf("Record %shas wrong checksum, computed: %lX\n", line, ComputedChecksum); /* notify line wrong in checksum */
    }
  }
  fclose(fRecord);
}

/* Part 4 */
int main()
{
  /* Display the menu of programing */
  printf("\n**********************************|| S-Record Application ||************************************\n");
  printf("**                                                                                            **\n");
  printf("**            Feature 1. Press '1' Convert Data Integer Into SREC File                        **\n");
  printf("**            Feature 2. Press '2' Convert 1 Specific File Binary Into SREC File              **\n");
  printf("**            Feature 3. Press '3' Convert 1-1 From Normal Binary File To SREC Format         **\n");
  printf("**            Feature 4. Press '4' Check Corruption For An S-Record file.                     **\n");
  printf("**            Feature 5. Press '5' Quit Program                                               **\n");
  printf("**                                                                                            **\n");
  printf("**********************************|| S-Record Application ||************************************\n");
  printf("\n");
  /* User Manual to help user can use conveniently.*/
  printf("=================================== User Manual=================================================================== \n");
  printf("***Please Read this below instruction before using the program !***\n");
  printf("***In Feature 1***\n");
  printf("-You can input from keyboard sequence of integer you want to convert to Record format and append in file \n");
  printf("-Please input sequence number in the following format : num1,num2,num3,...,numN and press enter to finish\n");
  printf("-The data will be converted and append in file : S_Record.srec\n");
  printf("-It is recommended that each time you enter from 1 to 100 numbers separated by commas\n");
  printf("\n");
  printf("***In Feature 2 and 3***\n");
  printf("-You input File Binary Path you want to convert\n");
  printf("-Note! :that the path does not contain special characters and must comply with the path format in C (Ex: C:/Users/Downloads/FileBinary.exe)\n");
  printf("-Program will convert data and append in file:(S_Record.srec) if you choose feature 2 and will write and save in file name(New_S_Record.srec) if you choose feature 3\n");
  printf("\n");
  printf("***In Feature 4***\n");
  printf("-You also input the file SRec path containing the data you want to check\n");
  printf("-Program will check in each line data and print out the calculated lines that are wrong compared to the checksum\n");
  printf("=================================== User Manual=================================================================== \n");

  while (1) /* loop to input choise in menu and call function to perform */
  {
    unsigned int iSelect; /* Declare variable to input choise in menu */
    printf("\nInput Select Feature : ");
    scanf(" %d", &iSelect);
    fflush(stdin);
    switch (iSelect)
    {
    case 1: /* call WriteforStudentInfile function */
      InputValueAndWriteInFileSRec();
      printf("\n\n\n\n\n");
      break;
    case 2:
      InputFileBinaryPath(0); /* flag = 0 to use feature 2*/
      printf("\n\n\n\n\n");
      break;
    case 3:
      InputFileBinaryPath(1); /* flag = 0 to use feature 3*/
      printf("\n\n\n\n\n");
      break;
    case 4:
      CheckForCorruption();
      printf("\n\n\n\n\n");
      break;
    case 5: /* Quit program */
      return 0;
    default:
      printf("\nInvalid option entered again !\n"); /* Notify Character invalid */
      break;
    }
  }

  getch();
  return 0;
}
