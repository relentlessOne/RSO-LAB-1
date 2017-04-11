#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<math.h>
#include<stdbool.h>
#include <time.h>

static int socket_desc , client_sock , c , read_size;
static struct sockaddr_in server , client;
static unsigned char client_message[2000];

static double changeBEdoubleToLEdouble(unsigned char* BEDouble){
    double result ;
    unsigned char *p = (unsigned char *)&result;

    p[0]  = BEDouble[7];
    p[1]  = BEDouble[6];
    p[2]  = BEDouble[5];
    p[3]  = BEDouble[4];
    p[4]  = BEDouble[3];
    p[5]  = BEDouble[2];
    p[6]  = BEDouble[1];
    p[7]  = BEDouble[0];

    return result;
}

static void prepareDoubleToSend(bool isBigEndian, double *pWhatToCheck,  unsigned char* out ){

  unsigned char *p = (unsigned char *)pWhatToCheck;

  printf("type 'double' object at address %p has value of %.2f\n",
  (void*)pWhatToCheck, *pWhatToCheck);

  size_t i = 0;

  for(; i < sizeof pWhatToCheck; ++i){
    printf("byte %u at address %p == %X\n",
    (unsigned int)i, (void*)(p + i), p[i]);
  }

  if(!isBigEndian){
    puts("Performing swipe operation");

    size_t i = sizeof pWhatToCheck;

    out[0] = p[7];
    out[1] = p[6];
    out[2] = p[5];
    out[3] = p[4];
    out[4] = p[3];
    out[5] = p[2];
    out[6] = p[1];
    out[7] = p[0];

      i = 0;

    for(; i < sizeof pWhatToCheck; ++i){
      printf("byte %u at address %p == %X\n",
      (unsigned int)i, (void*)(out + i), out[i]);
    }

    puts("Swipe done");

  } else {
      puts("Double is stored in big-endian architecture no swipe operation is needed");
      out[0] = p[0];
      out[1] = p[1];
      out[2] = p[2];
      out[3] = p[3];
      out[4] = p[4];
      out[5] = p[5];
      out[6] = p[6];
      out[7] = p[7];
  }
}

static void prepareIntToSend(bool isBigEndian, int *pWhatToCheck,  unsigned char* out ){

  unsigned char *p = (unsigned char *)pWhatToCheck;

  printf("type 'integer' object at address %p has value of %d\n",
  (void*)pWhatToCheck, *pWhatToCheck);

  size_t i = 0;

  for(; i < sizeof(int); ++i){
    printf("byte %u at address %p == %X\n",
    (unsigned int)i, (void*)(p + i), p[i]);
  }

  if(!isBigEndian){
    puts("Performing swipe operation");

    size_t i = 0;



    out[0] = p[3];
    out[1] = p[2];
    out[2] = p[1];
    out[3] = p[0];




    for(; i < sizeof(int); ++i){
      printf("byte %u at address %p == %X\n",
      (unsigned int)i, (void*)(out + i), out[i]);
    }

    puts("Swipe done");

  } else {
      puts("Integer is stored in big-endian architecture no swipe operation is needed");
      out[0] = p[0];
      out[1] = p[1];
      out[2] = p[2];
      out[3] = p[3];

  }
}

static bool isBigEndian(){
  uint32_t num = 0x12345678;
  uint8_t *ptr = (uint8_t *) &num;

  if (ptr[0] == 0x78){
    puts("CPU architecture is little-endian");
    return false;
  }
  else{
    puts("CPU architecture is big-endian");
    return true;
  }

}

static bool createSocket(){
  //Create socket
  socket_desc = socket(AF_INET , SOCK_STREAM , 0);
  if (socket_desc == -1)
  {
      printf("Could not create socket");
      return false;
  }
  puts("Socket created");

  //Prepare the sockaddr_in structure
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons( 8888 );
  return true;
}

static bool bindAndAcceptConnection(){
  //Bind
  if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
  {
      //print the error message
      perror("Bind failed. Error");
      return false;
  }
  puts("Bind done");

  //Listen
  listen(socket_desc , 3);

  //Accept and incoming connection
  puts("Waiting for incoming connections...");
  c = sizeof(struct sockaddr_in);

  //accept connection from an incoming client
  client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
  if (client_sock < 0)
  {
      perror("Accept failed");
      return false;
  }
  puts("Connection accepted");
  return true;
}

int main(int argc , char *argv[])
{

    if(!createSocket()) return 0;
    if(!bindAndAcceptConnection()) return 0;


    while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 )
    {


        unsigned char operationID[5];
        unsigned char rqID[5];

        memcpy(operationID,&client_message[0],4);
        operationID[4] = '\0';

        memcpy(rqID,&client_message[4],4);
        rqID[4] = '\0';


        if(operationID[0] == '0' && operationID[1] == '0' && operationID[2] == '0' && operationID[3] == '1'){
          unsigned char test [8];
          memcpy(test,&client_message[8],8);
          double recived = changeBEdoubleToLEdouble(&test[0]);
          printf("OPERATION_ID:%s RQ_ID:%s DOUBLE RECIVED:%.2f\n",operationID,rqID,recived);
          puts("Preforming sqrt operation and sending the data back to client");
          unsigned char backToClient[16];
          unsigned char backOperationID[4] = {'1','0','0','1'};
          unsigned char sqrtDoubleToSend[8];
          double sqrtRecived = sqrt(recived);
          prepareDoubleToSend(isBigEndian(),&sqrtRecived,&sqrtDoubleToSend[0]);
          int i = 0;
          for(;i<4;++i){
            backToClient[i] = backOperationID[i];
          }
          for(;i<8;++i){
            backToClient[i] = rqID[i-4];
          }
          for(;i<15;++i){
            backToClient[i] = sqrtDoubleToSend[i-8];
          }
          write(client_sock , backToClient , sizeof(backToClient));
          puts("Data sended");
        }

        if(operationID[0] == '0' && operationID[1] == '0' && operationID[2] == '0' && operationID[3] == '2'){
          printf("OPERATION_ID:%s RQ_ID:%s \n",operationID,rqID);
          puts("Acquiring server time and sending the data back to client");

          time_t rawtime;
          struct tm * timeinfo;

          time ( &rawtime );
          timeinfo = localtime ( &rawtime );
          char* timeDateToSend = asctime (timeinfo);
          printf ( "Current local time and date: %s", timeDateToSend );
          int length = strlen(timeDateToSend);
          unsigned char intToSend[4] ;
          prepareIntToSend(isBigEndian(),&length,&intToSend[0]);


          size_t size = 12 + length;
          unsigned char backToClient[size];
          unsigned char backOperationID[4] = {'1','0','0','2'};
          int i = 0;
          for(;i<4;++i){
            backToClient[i] = backOperationID[i];
          }
          for(;i<8;++i){
            backToClient[i] = rqID[i-4];
          }
          for(;i<12;++i){
            backToClient[i] = intToSend[i-8];
          }

          for(;i<size-1;++i){
            backToClient[i] = timeDateToSend[i-12];
          }

          write(client_sock , backToClient , sizeof(backToClient));
          puts("Data sended");


        }

    }

    if(read_size == 0)
    {
        puts("Client disconnected");
        puts("Server terminated");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("Recv failed");
    }

    return 0;
}
