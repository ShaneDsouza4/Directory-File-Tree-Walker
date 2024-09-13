
#define _XOPEN_SOURCE 700 //Allows necessary POSIX functions in 700 std like nftw are available
#include <stdio.h> //printf
#include <stdlib.h> //EXIT_fAILURE EXIT_SUCCESS
#include <ftw.h> //ftw()
#include <sys/types.h> // Data types used in system calls.
#include <sys/stat.h> //For stat(), S_ISDIR() Macro
#include <unistd.h>  //Access to the POSIX operating system API.
#include <string.h> //For string methods
#include <stdint.h> // Include for int64_t
#include <limits.h> //For PATH_MAX
#include <fcntl.h>
#include <errno.h> //For errno
#include <stdbool.h> //boolean

//Global Variables
int totalFileCount = 0;
int totalDirCount = 0;
long int totalFileSize = 0;
char *extToAvoid = NULL;
char sourceDir[PATH_MAX];
char destinationDir[PATH_MAX];


// Utility Function to chck if a directory existss
bool checkDirExist(char *path) {
    struct stat directoryInfo;

    // Stat func gets directory info
    // S_ISDR macro check the deirectory mode to determinee if it's a directory
    return (stat(path, &directoryInfo) == 0 && S_ISDIR(directoryInfo.st_mode)) ? true : false; 
}



// Utility Function to construct a destinationPath path, that will be used to match the structure.
void destPathCreation(char *destinationPath, const char *destinationDir, const char *filePath, const char *sourceDir) {
   
    // Copies the destination directory to destination path 
    // example: /home/dsouza56/assignment/folderNew
    strcpy(destinationPath, destinationDir);
    
    
    // Adds '/' to the destinatin path end
    // example: /home/dsouza56/assignment/folderNew/
    strcat(destinationPath, "/");

    //Addig the relative path by appending the file path, skipping the sourceDir part
    // example: 
    // Dest: /home/dsouza56/assignment/folder2 = 33 + 1 = 34 length
    // In file path /home/dsouza56/assignment/folder2/a1.c the pointer mpovs to the 34 character = a1.c
    // Now Dest PAth: /home/dsouza56/assignment/folderNew/a1.c
    strcat(destinationPath, filePath + strlen(destinationDir) + 1);
}


// Func reads souce file and writes to destination file
int copyFile(const char *sourcePath, const char *destinationPath){

    // Opens File Descriptor in Read Only from the source file
    int fdSrc = open(sourcePath, O_RDONLY); 

    if (fdSrc == -1) { // If can not open
        printf("Error whille opening the file.\n");
        exit(EXIT_FAILURE);
    }

    // Creating Destination files
    umask(0000); // Resetting umask to allow permissions

    // Creating file at destination  with all permission and Write Only
    int fdDest = open(destinationPath, O_CREAT| O_WRONLY, 0777); 

    if (fdDest == -1) { // If cannot create the file
        printf("\nError while creating the destination file.");
        exit(EXIT_FAILURE);
    }

    // In order to know about the Source File Descriptor, its info is needed using the fstat() func
    struct stat fileInfo;

    if(fstat(fdSrc, &fileInfo) == -1 ){ // If error occurs
        printf("Cannot get file '%s' details.\n", fdSrc);
        exit(EXIT_FAILURE);
    }

    // Creating buffer to add the the file contents
    // Allocating memory dynamically using the malloc function and st_size field in the FileInfo structure
    char *buffer = malloc(fileInfo.st_size); 

    if (buffer == NULL) { //If error occurs
        printf("Cannot allocate memory for '%s'\n", fdSrc);
        exit(EXIT_FAILURE);
    }


    // Read from source, adds to the buffer and reads the bytes we extracted
    long int n1;
    n1 = read(fdSrc, buffer, fileInfo.st_size); 
    //printf("The number of bytes read were %ld\n", n1);

    if (n1 == -1) { // If cannot read the file
        printf("Error in reading file '%s'\n", fdSrc);        
    } 
     
    // Writing the bytes "n1" from the buffer that holds it, to the destinationPath
    long int n2; 
    n2 = write(fdDest, buffer, n1);
    //printf("Number of bytes written are: %ld\n", n1);

    if (n2 == -1) { // If cannot write to the file
        printf("Error in writing file '%s'\n", fdDest);        
    } 

    // Close the open file descriptors & free the bufferr
    close(fdSrc);
    close(fdDest);
    free(buffer);

    return 0;
}


// Below are nftw callback funcs that take 4 arguments which is expcted by the nftw function.
// filePath is the absolute path pased during traversal.
// FileInfo cotains inforation of the file.
// Flag determines if it's a File or Directory.
// ftwInfo contains additional info, like depth of file..

// Functon to count the files in the Root Directory.
int countFiles(const char *filePath, const struct stat *FileInfo, int flag, struct FTW *ftwInfo){
    //printf("%s\n", filePath);

    if(flag == FTW_F){ // Checks if is a file. FTW_F is a constant used to identify file
       //printf("File found: %s\n", filePath);

        totalFileCount++; // Global variable usage to maintain count
    }

    return 0; 
}

// Function to count directories, presnt in the entire root directory
int countDirectories(const char *filePath, const struct stat *FileInfo, int flag, struct FTW *ftwInfo){
    //printf("%s\n", filePath);
    
    if(flag == FTW_D){ //Checks if is a Directory 
        //printf("Directory found: %s\n", filePath);

        // Increases the count in the gloval variable
        totalDirCount++;
    }

    return 0; 
}

//Func to calcuate the size of all files in bytes presnt in the entire root directory
int calcFileSize(const char *filePath, const struct stat *FileInfo, int flag, struct FTW *ftwInfo){
    //printf("%s\n", filePath);

    if(flag == FTW_F){ // Checks if is a File

       // Adds the size
       totalFileSize += (*FileInfo).st_size;
    }
    return 0; 
}

// This funciton traverses and copies the files and drectories wrt structure
int copyDirStructure(const char *filePath, const struct stat *FileInfo, int flag, struct FTW *ftwInfo) {
    

    // Charactr Array with PATH_MAX, as cannot determine the size of path
    char destinationPath[PATH_MAX]; 

    // Passing to function to create the destinationPath path, as structure needs to be maintained
    destPathCreation(destinationPath, destinationDir, filePath, sourceDir);

    //printf("Destination Path Finalized: %s\n", destinationPath);
    

    //Checking if the path is a directory
    if (flag == FTW_D) {
        //printf("Directory to Create: %s\n", destinationPath);
        umask(0000); //resetting umask to allow full permissions

        // Creating directory with full permissions
        if(mkdir(destinationPath, 0777) == -1){ //If error occurs 
            // Check if Failed due to existing already, if did keep going else terminate
            if(errno != EEXIST){ 
                printf("Destination directory '%s' cannot be created.\n", destinationPath);
                exit(EXIT_FAILURE);
            }

        }else{
            //printf("Destination directory '%s'  created.\n", destinationPath);
        }
        //printf("Exit the copying of directory\n");
        

    }
    else if (flag == FTW_F) { // Checking if the path is a File.

        //Extracting the extension from the file path received
        const char *pathExt = NULL; 
        
        //Running loop in reveres till length of the file path
        for(int i = strlen(filePath); i>=0; i--){

            // Identifying the first dot from reverse
            if(filePath[i] == '.'){ //abc.pdf => .pdf
                //Store in the pathExt variable
                pathExt = &filePath[i];

                break; // Exit the loop
            }
        }

        // Comparing to check if the path extension is same as the extensions to avoid
        if(extToAvoid != NULL && strcmp(pathExt, extToAvoid) == 0){
            printf("Skipped file '%s' with extension %s\n", filePath, extToAvoid);
            return 0; //skips file and mves to next
        }
        
        // Passes destination and file path to copyFile Function
        if (copyFile(filePath, destinationPath) == -1) { // If not copied succesfully
            printf("Cannot create File '%s'\n", destinationPath);
            exit(EXIT_FAILURE);
        }

    }
    return 0;
}

// Callback function for nftw to delete files and directories
int deleteSrcDir(const char *filePath, const struct stat *FileInfo, int flag, struct FTW *ftwInfo) {
    //printf("%s\n", filePath);

    // Utiliizing the remove function to delete all the files & direcories
    if (remove(filePath) == -1) { //If error occurs while removicng
        printf("Could not remove '%s'\n", filePath);
        exit(EXIT_FAILURE); 
    }

    return 0; 
}

// Function Traverses an entire directory including it subdirectory
// Takes a source path, and function as parameters
void nftwTraversal(char *path, int(*fn)()){

    // Will utilize the nftw() stytem call to recursibley traverse the directory
    // Runs through each file and directory
    // nftw begins traversing from the path
    // callback function which will be called by nftw for each traversal to perform an operation
    // Total number of file descriptors that can be used at same time, set to 50 
    // fLAG TO INdicate the behavious. FTW_PHYS indicates that symbolic links should be not counted, and stays within directory structur
    if(nftw(path, fn, 50, FTW_PHYS) == 0 ){
        //printf("Traversal In Process!");
    }else{ 
        // If error occurs while traversing
        printf("Error occurred while visiting '%s.\n", path);
        exit(EXIT_FAILURE); 
    }
}


int main(int argc, char *argv[]){
    //printf("Arg1: %s\n", argv[1]);
    //printf("Arg2: %s\n", argv[2]);
    //printf("Arg3: %s\n", argv[3]);
    //printf("Arg4: %s\n", argv[4]);

    // If user provids 3 arguments will cater to Count of Files, Directory, and File size. 
    if(argc == 3){

        // Command 1: to display count of all Files present in the root directory
        if(strcmp(argv[1], "-nf") == 0){ // Compairing to check if user entered -nf

            // Check if user provided the correct absolute Path that exists in the system.
            if (checkDirExist(argv[2])) {
                //printf("Checking for files in directory: %s.\n", argv[2]);
        
                // Passing argv[2] as path and countFiles function as args to nftwTraversal()to begin traversal
                nftwTraversal(argv[2], countFiles);

                // Displaying result
                printf("Total number of present files: %d\n", totalFileCount);
                exit(EXIT_SUCCESS);

            } else {

                // Show error message tht path not present
                printf("Directory '%s' is not present in the system.\n", argv[2]);
                exit(EXIT_FAILURE);

            }
        }// Command 2: to display count of all Directories present in the root directory
        else if(strcmp(argv[1], "-nd") == 0){ //C omparison check for -nd

            // Checking if the root directory exists in the system
            if (checkDirExist(argv[2])) { 
                //printf("Checking for directories in %s directory.\n", argv[2]);

                // Passing path and countDirectories fun to the nftwTraversal func to begin traversal process
                nftwTraversal(argv[2], countDirectories);

                // Results 
                // -1 to subtract the root directory.
                printf("Total number of directories: %d\n", totalDirCount - 1); 
                exit(EXIT_SUCCESS);

            } else {
                // Root Directory does not exist
                printf("Directory '%s' is not present in the system.\n", argv[2]);
                exit(EXIT_FAILURE);
            }

        }// Command 3: to list size in bytes of all files present in the root directory
        else if(strcmp(argv[1], "-sf") == 0){ //Comparison check for -sf
           
            // Checking if the directory exists in the system
            if (checkDirExist(argv[2])) {
                //printf("Checking for files in %s directory.\n", argv[2]);
                
                // Passing path and calcFileSize function to nftwTraversal() to bring traversal 
                nftwTraversal(argv[2], calcFileSize);
    
                // Results
                printf("Size of all files: %ld bytes.\n", totalFileSize);
                exit(EXIT_SUCCESS);

            } else {
                //Directory does not exist
                printf("Directory '%s' is not present in the system.\n", argv[2]);
                exit(EXIT_FAILURE);
            }
        }else{ // 3 argument operations are not -nf, -nd, -sf
            printf("Invalid operation argument '%s' passed.\n", argv[1]);
            exit(EXIT_FAILURE);
        }
    }
    else if(argc == 4 || argc ==5){ //If user provid 4-5 arguments will cater to Copying and Moving a Directory.
        
        // Command 4: Copy whole subdirectory from sourcePath path to destintation path
        if(strcmp(argv[1],"-cpx") == 0 ){

            // Check if an extension is provided to be avoided
            if(argv[4] != NULL){ // If nothing is provided, considered NULL.
                extToAvoid = argv[4];

                // Check which file to avoid as can only be one of these 3 extenstions. .txt, .c, .pdf
                if (strcmp(extToAvoid, ".txt") != 0 && strcmp(extToAvoid, ".pdf") != 0 && strcmp(extToAvoid, ".c") != 0) {
                    printf("Invalid extension '%s'. Please enter '.txt', '.pdf', or '.c'.\n", extToAvoid);
                    exit(EXIT_FAILURE);
                }
                //printf("Ext received: %s\n", extToAvoid);

            }
            
            // Check if Source Directory exists
            if (checkDirExist(argv[2])) {
                strcpy(sourceDir, argv[2]); // Assigning Path to Source Directory
                strcpy(destinationDir, argv[3]); // Assigning Path to Destination Directory

                // Passing path and copyDirStructure function to nftwTraversal() start traversal process
                nftwTraversal(argv[2], copyDirStructure);
                
                // Results
                printf("Source Directory copied succesfully!\n");
                exit(EXIT_SUCCESS);

            } else {
                // Directory does not exist
                printf("Directory '%s' is not present in the system.\n", argv[2]);
                exit(EXIT_FAILURE);

            }

        }// Command 5: Moving whole directory from source to destination path
        else if(strcmp(argv[1],"-mv")== 0){
            //printf("1. Time to move!\n");

            // Check if source directory exists
            if(checkDirExist(argv[2])){

                //Assigning values
                strcpy(sourceDir, argv[2]);
                strcpy(destinationDir, argv[3]);

                // Pass Source Path and copyDirStructure() to nftwTraversal() to begin traversing
                nftwTraversal(argv[2], copyDirStructure);

                //Removing the Source directory
                //FTW_DEPTH starts traversal from the contents and then reaches the Source Directory
                if(nftw(argv[2], deleteSrcDir, 50, FTW_DEPTH | FTW_PHYS ) == -1){ //If error occurs
                    printf("Error occurred while visiting the directory '%s'\n", argv[2]);
                    exit(EXIT_FAILURE); 
                }

                // Results
                printf("Source Directory '%s' Moved to '%s'\n", sourceDir, destinationDir );
                exit(EXIT_SUCCESS);
                

            }else{
                //If Source directory is not available
                printf("Directory '%s' is not present in the system.\n", argv[2]);
            }
        }else{ // 2 argument operations are not -cpx, -mv
            printf("Invalid operation argument '%s' passed.\n", argv[1]);
            exit(EXIT_FAILURE);
        }
    }else{//None of the arguments user entered are valid
        printf("Invalid arguments provided. Please enter again.\n");
        exit(EXIT_FAILURE);
    }

    return 0; 
}