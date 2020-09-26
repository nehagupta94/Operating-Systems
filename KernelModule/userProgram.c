#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
 
#define BUFFER_LENGTH 256000           
static char receive[BUFFER_LENGTH];    
 
int main(){
   int ret, fd;
   printf("Starting device test code...\n");
   fd = open("/dev/process_list", O_RDWR);        
   if (fd < 0){
      perror("Failed to open the device...");
      return errno;
   }

   printf("Reading from the device...\n");
   ret = read(fd, receive, BUFFER_LENGTH);       
   if (ret < 0){
      perror("Failed to read the message from the device.");
      return errno;
   }
   printf("The received message is: [%s]", receive);
   close(fd);
   printf("End of the program\n");
   return 0;
}
