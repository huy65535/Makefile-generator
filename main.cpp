#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <malloc.h>

#define MAX_DIR_PATH 	256
#define MAX_BUF 		256

void enterDirPath(char dirPath[])
{
	printf("Enter folder path: ");
	gets(dirPath);
	
	if (dirPath[strlen(dirPath) - 1] != '/' && dirPath[strlen(dirPath)] != '\\')
		strcat(dirPath, "\\");
}

void upcase(char str[])
{
	for (int i = 0; str[i] != '\0'; i++)
	{
		if (str[i] >= 'a' && str[i] <= 'z')
			str[i] -= 32;
	}
}

bool isCodeFile(char name[])
{
	int pos = 0;
	for (pos = strlen(name) - 1; pos >= 0; pos--)
		if (name[pos] == '.') break;
	
	char temp[MAX_BUF];
	strcpy(temp, "");
	
	if (pos >= 0)
	{
		for (int i = pos; i < strlen(name); i++)
		{
			strcat(temp, " ");
			temp[strlen(temp) - 1] = name[i];
		}
		
		if (strcmp(temp, ".H") == 0 || strcmp(temp, ".CPP") == 0)
			return 1;
			
	}
	
	return 0;
}

bool getFileList(char dirPath[], char **&fileList, int &fileCount)
{
	DIR *dir;
	struct dirent *entry;
	
	if (!(dir = opendir(dirPath)))
		return 0;
	
	fileCount = 0;
	char temp[MAX_BUF];
	
	while (entry = readdir(dir))
	{
		if (entry->d_type == DT_DIR)
			continue;
		
		strcpy(temp, entry->d_name);
		upcase(temp);
	
		if (isCodeFile(temp))
		{
			//puts(temp);
			fileCount++;
			fileList = (char **) realloc(fileList, fileCount * sizeof(char *));
			fileList[fileCount - 1] = strdup(temp);
		}
	}
	
	return 1;
}

void getObjList(char **fileList, int fileCount, char **&objList, int &objCount)
{
	objCount = 0;
	char temp[MAX_BUF];
	
	for (int i = 0; i < fileCount; i++)
	{
		int dotPos = strstr(fileList[i], ".H\0") - fileList[i];
		
		if (dotPos < 0 || dotPos >= strlen(fileList[i]))
			dotPos = strcmp(fileList[i], "MAIN.CPP\0") == 0 ? 4 : -1;
		
		if (dotPos >= 0 && dotPos < strlen(fileList[i]))
		{
			objCount++;
			objList = (char **)realloc(objList, objCount * sizeof(char *));
			
			strcpy(temp, fileList[i]);
			temp[dotPos] = '\0';
			strcat(temp, ".O");
			
			objList[objCount - 1] = strdup(temp);
		}
	}
}

void freedom(char **&fileList, int fileCount, char **&objList, int objCount)
{
	for (int i = 0; i < fileCount; i++)
		free(fileList[i]);
	
	for (int i = 0; i < objCount; i++)
		free(objList[i]);
	
	free(fileList);
	free(objList);
}

void getOutputPath(char outputPath[], char dirPath[])
{
	strcpy(outputPath, dirPath);
	strcat(outputPath, "Makefile");
}

bool writeMakefile(char dirPath[], char **fileList, int fileCount, char **objList, int objCount)
{
	char outputPath[strlen(dirPath) + 8 + 1];
	getOutputPath(outputPath, dirPath);
	
	FILE *fo = fopen(outputPath, "w");
	if (!fo) return 0;
	
	fprintf(fo, "CC = g++\n\nall: ");
	for (int i = 0; i < objCount; i++)
		fprintf(fo, "%s ", objList[i]);
	fprintf(fo, "\n\t$(CC) ");
	for (int i = 0; i < objCount; i++)
		fprintf(fo, "%s ", objList[i]);
	fprintf(fo, "-o main\n\n");
	for (int i = 0; i < objCount; i++)
	{
		fprintf(fo, "%s: ", objList[i]);
		char tempCPP[MAX_BUF], tempH[MAX_BUF];
		strcpy(tempCPP, objList[i]);
		tempCPP[strlen(tempCPP) - 2] = '\0';
		strcpy(tempH, tempCPP);
		strcat(tempCPP, ".CPP");
		strcat(tempH, ".H");
		
		for (int j = 0; j < fileCount; j++)
		{
			if (strstr(fileList[j], tempCPP) || strstr(fileList[j], tempH))
				fprintf(fo, "%s ", fileList[j]);
		}
		if (strcmp(objList[i], "MAIN.O") == 0)
		{
			for (int j = 0; j < fileCount; j++)
				if (strstr(fileList[j], ".H\0"))
					fprintf(fo, "%s ", fileList[j]);
		}
		fprintf(fo, "\n\t$(CC) -g -c %s\n\n", tempCPP);
		
	}
	fprintf(fo, "clean:\n\trm -f *.o\n\trm main");
	
	fclose(fo);
	
	return 1;
}

bool createMakefile(char dirPath[])
{
	int fileCount, objCount = 0;
	char **fileList = NULL, **objList = NULL;
	
	if (!getFileList(dirPath, fileList, fileCount))
		return 0;
	
	getObjList(fileList, fileCount, objList, objCount);

	if (!writeMakefile(dirPath, fileList, fileCount, objList, objCount))
		return 0;
		
	freedom(fileList, fileCount, objList, objCount);
	
	return 1;
}

int main()
{
	char dirPath[MAX_DIR_PATH];
	enterDirPath(dirPath);
	
	if (createMakefile(dirPath))
		printf("Successfully!");
	else
		printf("Failed.");
	
	return 0;
}