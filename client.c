#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include <stdbool.h>



static int socket_desc;
static struct sockaddr_in server;
static unsigned char operationID[4] = {'0','0','0','1'};
static unsigned char rqID[4] = {'0','0','0','1'};
static unsigned char doubleToSend[8];
static unsigned char dataToSend[16];
static unsigned char server_reply[2000];


static bool createSocket(){
  socket_desc = socket(AF_INET , SOCK_STREAM , 0);
  if (socket_desc == -1)
  {
      puts("Could not create socket");
      return false;
  }

  puts("Socket Created");
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons( 8888 );

  return true;
}

static bool connectToRemoteServer(){
  if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
  {
      puts("connect error");
      return false;
  }
  puts("Connected");
  return true;
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

static double changeBEintToLEint(unsigned char* BEInt){
    int result ;
    unsigned char *p = (unsigned char *)&result;

    p[0]  = BEInt[3];
    p[1]  = BEInt[2];
    p[2]  = BEInt[1];
    p[3]  = BEInt[0];

    return result;
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

static bool sendFirstRequest(double whatToSend){


  prepareDoubleToSend(isBigEndian(),&whatToSend,&doubleToSend[0]);

  int i = 0;

  for(;i< 4;++i){
      dataToSend[i] = operationID[i];
  }

  for(; i< 8; ++i){
    dataToSend[i] = rqID[i-4];
  }

  for(; i< 15; ++i){
    dataToSend[i] = doubleToSend[i-8];
  }

  if( send(socket_desc , dataToSend , sizeof(dataToSend) , 0) < 0)
  {
      puts("Send failed");
      return false;
  }

  puts("First request has been sended");
  return true;



}

static bool reciveFirstRequest(){
  if( recv(socket_desc, server_reply , 2000 , 0) < 0)
  {
      puts("recv failed");
      return false;
  }
  puts("Reply received");

  unsigned char recivedOperationID[5];
  unsigned char recivedRqID[5];
  unsigned char recivedDouble[8];
  double sqrtValue;

  memcpy(recivedOperationID,&server_reply[0],4);
  recivedOperationID[4] = '\0';
  memcpy(recivedRqID,&server_reply[4],4);
  recivedRqID[4] = '\0';
  memcpy(recivedDouble,&server_reply[8],8);
  sqrtValue = changeBEdoubleToLEdouble(&recivedDouble[0]);
  printf("OPERATION_ID:%s RQ_ID:%s SQRT_VALUE:%.2f\n",recivedOperationID, recivedRqID,sqrtValue);
  return true;
}

static bool sendSecondRequest(){
  operationID[3] = '2';
  rqID[3] = '2';

  int i = 0;

  for(;i< 4;++i){
      dataToSend[i] = operationID[i];
  }

  for(; i< 8; ++i){
    dataToSend[i] = rqID[i-4];
  }

  if( send(socket_desc , dataToSend , sizeof(dataToSend) , 0) < 0)
  {
      puts("Send failed");
      return false;
  }

  puts("Second request has been sended");
  return true;

}

static bool reciveSecondRequest(){
  if( recv(socket_desc, server_reply , 2000 , 0) < 0)
  {
      puts("recv failed");
      return false;
  }
  puts("Reply received");

  unsigned char recivedOperationID[5];
  unsigned char recivedRqID[5];
  unsigned char recivedTimeDateStrlen[5];
  int timeDateStrln;

  memcpy(recivedOperationID,&server_reply[0],4);
  recivedOperationID[4] = '\0';

  memcpy(recivedRqID,&server_reply[4],4);
  recivedRqID[4] = '\0';

  memcpy(recivedTimeDateStrlen,&server_reply[8],4);
  recivedTimeDateStrlen[4] = '\0';

  timeDateStrln = changeBEintToLEint(&recivedTimeDateStrlen[0]);
  unsigned char timeDateString[timeDateStrln];
  memcpy(timeDateString,&server_reply[12],timeDateStrln);
  timeDateString[timeDateStrln-1] = '\0';

  printf("OPERATION_ID:%s RQ_ID:%s DATE_TIME_STRING_LENGTH:%d DATE_TIME:%s\n",recivedOperationID,recivedRqID,timeDateStrln,timeDateString );
  return true;
}



int main(int argc , char *argv[])
{

    if(!createSocket()) return 0;
    if(!connectToRemoteServer()) return 0;
    if(!sendFirstRequest(222.2123122)) return 0;
    if(!reciveFirstRequest()) return 0;
    if(!sendSecondRequest()) return 0;
    if(!reciveSecondRequest()) return 0;

    puts("Client terminated");

    return 0;
}
