// CODE FOR PROGRAMMING STM8L050J3 WITH THE CODE CONTAINED IN STM8L.H HEADER.
// THE LATTER CAN BE GENERATED FROM THE S19 FILE USING THE s19toheader PYTHON SCRIPT
// TESTED ON GAPPOC- B V2.1
// AUTHOR : M.GUERMANDI - GWT

#include "rt/rt_api.h"
#include "pmsis.h" 
#include "stm8l050j3.h"
#include "stm8l_data.h"

#define BIT_STREAM_SIZE 1024

#define GPIO_NUMBER 15

#define VERBOSE     1


enum SWIM_CMD {SWRST, ROTF, WOTF};


uint8_t bit_stream[BIT_STREAM_SIZE];

// Function to get parity of number n. It returns 1 
// if n has odd parity, and returns 0 if n has even 
// parity  
uint8_t getParity(uint8_t n) 
{ 
    uint8_t parity = 0; 
    while (n) 
    { 
        parity = !parity; 
        n     = n & (n - 1); 
    }      
    return parity; 
} 

void delay_ms(int ms)
{
  uint8_t volatile ccc=0;

  for(int i=0;i<50000*ms;i++) 
        ccc++;
}


//INITITIALIZE SWIM CLOCK AT 4 KHz FOR ENTRY SEQUENCE
void init_SWIM_clock_4KHz()
{
  rt_freq_set(RT_FREQ_DOMAIN_FC, 248000000);
  timer_cmp_set(timer_base_fc(0, 0), 62500); 

  timer_conf_set(timer_base_fc(0, 0),
    TIMER_CFG_LO_ENABLE(1) |   // Enabled timer
    TIMER_CFG_LO_RESET(1)  |   // Reset it
    TIMER_CFG_LO_IRQEN(1)  |   // Activate interrupt
    TIMER_CFG_LO_MODE(1)   // Clear when it reaches compare value
  );
}

//INITITIALIZE SWIM CLOCK AT 4 MHz FOR OPERATIONS
void init_SWIM_clock_4MHz()
{
  rt_freq_set(RT_FREQ_DOMAIN_FC, 248000000);
  timer_cmp_set(timer_base_fc(0, 0), 30); // Compare value 250MHz/8MHz

  timer_conf_set(timer_base_fc(0, 0),
    TIMER_CFG_LO_ENABLE(1) |   // Enabled timer
    TIMER_CFG_LO_RESET(1)  |   // Reset it
    TIMER_CFG_LO_IRQEN(1)  |   // Activate interrupt
    TIMER_CFG_LO_MODE(1)   // Clear when it reaches compare value
  );
}


//ENTER SWIM SEQUENCE
void enter_SWIM()
{
  for (int i=0;i<16;i=i+4)
  {
    bit_stream[i] = 0;
    bit_stream[i+1] = 0;    
    bit_stream[i+2] = 1;
    bit_stream[i+3] = 1; 
  }
  for (int i=16;i<25;i=i+2)
  {
    bit_stream[i] = 0;
    bit_stream[i+1] = 1;    
  }

  hal_gpio_padout_set(0);
  hal_gpio_paddir_set(1<<GPIO_NUMBER);
  rt_time_wait_us(1000);


  // Activate core wakeup when received timer IRQ0
  eu_evt_maskSet(1<<10);

  for (int i=0; i<25; i++)
  {
    eu_evt_waitAndClr();
    //hal_gpio_padout_set(bit_stream[i]);
    hal_gpio_paddir_set(bit_stream[i]<<GPIO_NUMBER);
  }
  hal_gpio_paddir_set(0);
  eu_evt_maskClr(1<<10);
}

//PREPARE SEQUENCE FOR 1 BIT ON SWIM (HIGH SPEED)
void prepare_bit_SWIM_HS(uint32_t bit_index, uint8_t* bit_array, uint8_t value)  //VALUE 2 IS FOR HZ (FIXED AT 0 TO AVOID SHORTS AS IT IS A OPEN DRAIN LINE)
{

  uint32_t index = bit_index*10;
  
  if (value == 0)
  {
    for (int i=index;i<index+8;i++)
      bit_array[i]=1;
    bit_array[index+8]=0;
    bit_array[index+9]=0;  
  }
  else if (value ==1)
  {
    bit_array[index]=1;
    bit_array[index+1]=1;
    for (int i=index+2;i<index+10;i++)
      bit_array[i]=0;
  } 
  else
  {
    for (int i=index;i<index+10;i++)
      bit_array[i]=0;                       
  }  
}


//PREPARE SEQUENCE FOR 1 BIT ON SWIM (NORMAL SPEED)
void prepare_bit_SWIM(uint32_t bit_index, uint8_t* bit_array, uint8_t value) 
{

  uint32_t index = bit_index*22;
  
  if (value == 0)
  {
    for (int i=index;i<index+20;i++)
      bit_array[i]=1;
    bit_array[index+20]=0;
    bit_array[index+21]=0;  
  }
  else if (value ==1)
  {
    bit_array[index]=1;
    bit_array[index+1]=1;
    for (int i=index+2;i<index+22;i++)
      bit_array[i]=0;
  } 
  else
  {
    for (int i=index;i<index+22;i++)
      bit_array[i]=0;                       
  }  
}

//PREPARE SEQUENCE FOR 1 BYTE ON SWIM
void prepare_byte_SWIM(uint32_t bit_index, uint8_t* bit_array, uint8_t value)
{
  uint8_t current_bit;
  
  prepare_bit_SWIM(bit_index++, bit_array, 0);

  for(int i=0; i<8; i++,bit_index++)
  {
    current_bit=(0x1)&(value>>(7-i));
    prepare_bit_SWIM(bit_index, bit_array, current_bit);
  }

  prepare_bit_SWIM(bit_index++, bit_array, getParity(value));
  prepare_bit_SWIM(bit_index++, bit_array, 2);   //READ ACK
}

//PREPARE SEQUENCE FOR ACK BIT ON SWIM (WE SEND THE ACK REGARDLESS AS WE DON'T HAVE TIME TO CHECK IT. WE CAN ASK RETRANSMISSION IF SOMETHING IS WRONG)
void prepare_ACK_SWIM(uint32_t bit_index, uint8_t* bit_array)
{
  for(int i=0; i<10; i++,bit_index++)
    prepare_bit_SWIM(bit_index, bit_array, 0);

  prepare_bit_SWIM(bit_index, bit_array, 1);   
}

// PREAPRE SEQUENCE FOR SWIM COMMAND (SRST, WOTF, ROTF). RETURN NUMBER OF BITS IN THE STREAM (NOT IN THE SEQUENCE)
int prepare_SWIM_command(enum SWIM_CMD command, uint8_t* bit_array, uint32_t address, uint8_t number_of_bytes, uint8_t* data)
{

  uint8_t reg_h = address>>16;
  uint8_t reg_m = address>>8;
  uint8_t reg_l = address;

  prepare_bit_SWIM(0, bit_array, 0);

  switch (command)
  {
    case SWRST: //OOO PARITY 0
      prepare_bit_SWIM(1, bit_array, 0);
      prepare_bit_SWIM(2, bit_array, 0);
      prepare_bit_SWIM(3, bit_array, 0);
      prepare_bit_SWIM(4, bit_array, 0); 
      break;
    case ROTF: //OO1 PARITY 1
      prepare_bit_SWIM(1, bit_array, 0);
      prepare_bit_SWIM(2, bit_array, 0);
      prepare_bit_SWIM(3, bit_array, 1);
      prepare_bit_SWIM(4, bit_array, 1);
      break; 
    case WOTF: //O10 PARITY 1               
      prepare_bit_SWIM(1, bit_array, 0);
      prepare_bit_SWIM(2, bit_array, 1);
      prepare_bit_SWIM(3, bit_array, 0);
      prepare_bit_SWIM(4, bit_array, 1);
      break;
  }

  prepare_bit_SWIM(5, bit_array, 2); //READ ACK

  switch (command)
  {
    case SWRST: //NOTHING TO DO
      break;
    case ROTF:
    case WOTF: //WRITE REGISTER ADDRESS
      prepare_byte_SWIM(6, bit_array, number_of_bytes);
      prepare_byte_SWIM(17, bit_array, reg_h);
      prepare_byte_SWIM(28, bit_array, reg_m);
      prepare_byte_SWIM(39, bit_array, reg_l);        
  }

  if (command == WOTF)
    for (int i=0;i<number_of_bytes;i++)
      prepare_byte_SWIM(50+i*11, bit_array, data[i]);
  else
    for (int i=0;i<number_of_bytes;i++)
      prepare_ACK_SWIM(50+i*11, bit_array);       //HIGH IMPEDANCE + ACK

  return(50+number_of_bytes*11);    //RETURN NUMBER OF BITS IN THE STREAM

}

// PREPARE AND SEND SEEUNCE FOR WOTF
void send_WOTF(uint8_t* bit_array, uint32_t address, uint8_t number_of_bytes, uint8_t* data)
{ 
  int stream_length = prepare_SWIM_command(WOTF, bit_array, address, number_of_bytes, data)*22;

  // Activate core wakeup when received timer IRQ0
  eu_evt_maskSet(1<<10);

  for (int i=0; i<stream_length; i++)
  {
    eu_evt_waitAndClr();
    hal_gpio_paddir_set(bit_array[i]<<GPIO_NUMBER);
  }

  eu_evt_maskClr(1<<10);
}

// PREPARE AND SEND SEQUENCE FOR RESET
void send_SWRST(uint8_t* bit_array)
{
  prepare_SWIM_command(SWRST, bit_array, 0x00, 0, bit_array);

  // Activate core wakeup when received timer IRQ0
  eu_evt_maskSet(1<<10);

  for (int i=0; i<110; i++)
  {
    eu_evt_waitAndClr();
    hal_gpio_paddir_set(bit_array[i]<<GPIO_NUMBER);
  }

  eu_evt_maskClr(1<<10);

}

//WRITE REGISTER (SINGLE BYTE WOTF)
void write_REG(uint8_t* bit_array, uint32_t address, uint8_t* data)
{ 
   send_WOTF(bit_array, address, 1, data);
}

//RESET SWIM COMMUNICATION
void RESET_COMM()
{
// Activate core wakeup when received timer IRQ0
  eu_evt_maskSet(1<<10);

  for (int i=0; i<128; i++)
  {
    eu_evt_waitAndClr();
    hal_gpio_paddir_set(1<<GPIO_NUMBER);
  }
  
  hal_gpio_paddir_set(0);   //SWITCH TO INPUT

  for (int i=0; i<200; i++)
  {
    eu_evt_waitAndClr();
  }

  eu_evt_maskClr(1<<10);
}

//READ REGISTER (SINGLE BYTE ROTF) AND RETURNS PARITY
uint8_t read_REG_RAW(uint8_t* bit_array, uint32_t address, uint8_t* data)
{ 
  *data = 0;
  uint8_t parity,aa=0;

  prepare_SWIM_command(ROTF, bit_array, address, 1, data);

  // Activate core wakeup when received timer IRQ0
  eu_evt_maskSet(1<<10);

  for (int i=0; i<50*22; i++)
  {
    eu_evt_waitAndClr();
    hal_gpio_paddir_set(bit_array[i]<<GPIO_NUMBER);
  }
  

  eu_evt_waitAndClr();
  hal_gpio_paddir_set(0);   //SWITCH TO INPUT
  eu_evt_waitAndClr();
  eu_evt_waitAndClr();
  while(((hal_gpio_padin_get(0)>>GPIO_NUMBER)&(0x01))==1);  //WAIT TILL STM8L CLAIMS THE LINE

  for (volatile int i=0; i<22; i++)  //FIRST BIT IS ALWAYS AT 1
    eu_evt_waitAndClr();

  //READ BYTE
  for (int i=0; i<8; i++)
  {
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    *data = (*data<<1)+((hal_gpio_padin_get(0)>>GPIO_NUMBER)&(0x01));
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();

  }

  //PARITY CHECK

    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    parity=(hal_gpio_padin_get(0)>>GPIO_NUMBER)&(0x01);
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();
    eu_evt_waitAndClr();

  //SEND ACK 
  eu_evt_waitAndClr();
  hal_gpio_paddir_set(1<<GPIO_NUMBER);
  eu_evt_waitAndClr();
  eu_evt_waitAndClr();
  eu_evt_waitAndClr();
  eu_evt_waitAndClr();
  eu_evt_waitAndClr();
  hal_gpio_paddir_set(0);
  eu_evt_waitAndClr();
  eu_evt_waitAndClr();
  eu_evt_waitAndClr();
  eu_evt_waitAndClr();
  eu_evt_waitAndClr();
  eu_evt_waitAndClr();
  eu_evt_waitAndClr();
  eu_evt_waitAndClr();
  eu_evt_waitAndClr();
  eu_evt_waitAndClr();
  eu_evt_waitAndClr();
  eu_evt_waitAndClr();
  eu_evt_waitAndClr();
  eu_evt_waitAndClr();
  eu_evt_waitAndClr();
  eu_evt_waitAndClr();

  eu_evt_maskClr(1<<10);

  return(parity==getParity(*data));
}

// READ REGISTER AND LOOPS TILL PARITY CHECK IS OK
void read_REG(uint8_t* bit_array, uint32_t address, uint8_t* data)
{ 
  uint8_t count=0;

  while(read_REG_RAW(bit_array, address, data)==0)
  {
    count++;
    RESET_COMM();
    delay_ms(2);
  }
}


// UNLOCK PROGRAM MEMORY
void unlock_MASS_PROGRAM()
{
 uint8_t data, PUL=0;


  // WAIT FOR PUL BIT TO GO HIGH
  while (!PUL)
  {
    RESET_COMM();
    delay_ms(1);

    data=0x56;
    send_WOTF(bit_stream,FLASH_PUKR,1,&data);
  
    data = 0xAE;
    send_WOTF(bit_stream,FLASH_PUKR,1,&data);

    read_REG(bit_stream, FLASH_IAPSR, &data);
    PUL = (data&0x02)>>1;
#ifdef VERBOSE
    printf("IAPSR: %d\n", data);
#endif
  }
}

// LOCK PROGRAM MEMORY
void lock_MASS_PROGRAM()
{
  uint8_t data, PUL = 1;

  while(PUL)
  {
    RESET_COMM();
    read_REG(bit_stream, FLASH_IAPSR, &data);
    data = (data&0xFD);   //SET PUL TO ZERO
    send_WOTF(bit_stream,FLASH_IAPSR,1,&data);
    read_REG(bit_stream, FLASH_IAPSR, &data);
    PUL = (data&0x02)>>1;
#ifdef VERBOSE
    printf("IAPSR: %d\n", data);
#endif
  }
}

// PROGRAM FLASH BYTE BY BYTE AND RETURNS NUMBER OF LOCATIONS UNSUCCESSFULLY VERIFIED
int program_FLASH()
{
  uint8_t data, data_read;
  int addr;
  int errors=0;

  uint8_t tries;
  
  for (int i=0;i<stm8l_code_size*2;i=i+2)
  {
    tries=1;
    addr=stm8l_code_data[i];
    data=stm8l_code_data[i+1];
   
    RESET_COMM();

    write_REG(bit_stream,addr,&data);

    delay_ms(15);

    read_REG(bit_stream, addr, &data_read);

    while((data!=data_read)&&(tries<100))
    {
      tries++;
      RESET_COMM();
      write_REG(bit_stream,addr,&data);

      delay_ms(15);
 
      RESET_COMM();
      
      read_REG(bit_stream, addr, &data_read);
      
    }

    if(tries==100)
      errors++;


#ifdef VERBOSE
    if(tries==100)
      printf("COULDN'T WRITE %d %d %d %d\n\n",i,tries, data,data_read);

    if((i%1000)==0)
      printf("%d percent DONE\n",100*i/stm8l_code_size/2);
#endif

  }


  return(errors);
}

void init_board()
{

  //rt_pad_set_function(GPIO_NUMBER+4,1) ;
  
  //rt_gpio_init(0, GPIO_NUMBER);            

  pi_pad_set_function(PI_PAD_36_A15_I2S1_WS, PI_PAD_29_B34_GPIO_A15_FUNC1);
  pi_gpio_pin_configure(0, PI_GPIO_A15_PAD_29_B34, PI_GPIO_OUTPUT);

  rt_pad_set_function(7,1) ;   
  rt_gpio_init(3, 0);            

  //rt_pad_set_function(26,0) ;            
  //rt_pad_set_function(41,0) ;             

  hal_gpio_padout_set(0);

  pulp_write32(0x1A10101C,1<<GPIO_NUMBER);

  hal_gpio_paddir_set(1<<GPIO_NUMBER); 
}


void SWIM_init_sequence()
{
  uint8_t data=0xA0, data_read=0;

  init_SWIM_clock_4KHz();

  enter_SWIM();

  init_SWIM_clock_4MHz();

  delay_ms(10);

  while((data&0xFD)!=(data_read&0xFD))    //CAN READ 0xA0 or 0xA2
  {

    RESET_COMM();
    write_REG(bit_stream, SWIM_CSR, &data);

    delay_ms(10);

    read_REG(bit_stream, SWIM_CSR, &data_read);
#ifdef VERBOSE
    printf("CSR: %d\n", data_read); //160 or 162
#endif
  }
  //SW RESET
   send_SWRST(bit_stream); 
    
 
}

void reboot_STM8L()
{
  uint8_t data, data_read, RST=0;

  send_SWRST(bit_stream); 

  while(!RST) 
  {

    RESET_COMM();
    read_REG(bit_stream, SWIM_CSR, &data_read);
    
    data = 0xA4;
    delay_ms(10);
    RESET_COMM();
    write_REG(bit_stream, SWIM_CSR, &data);

    delay_ms(10);
    RESET_COMM();
    read_REG(bit_stream, SWIM_CSR, &data_read);
    
    RST = (data&0x04)>>2;

#ifdef VERBOSE
    printf("CSR: %d\n", data_read);
#endif
  }
  RESET_COMM();
  //SW RESET WITH SWIM OFF AT REBOOT
  send_SWRST(bit_stream);  
}


int main()
{

#ifdef VERBOSE
  printf("INIT BOARD\n");
#endif 
  init_board();


#ifdef VERBOSE
  printf("INIT SWIM\n");
#endif 
  SWIM_init_sequence();

  delay_ms(10);
   

#ifdef VERBOSE
  printf("UNLOCK MASS PROGRAM\n");
#endif 
  unlock_MASS_PROGRAM();
  
  delay_ms(10);

#ifdef VERBOSE
  printf("PROGRAMMING FLASH\n");
#endif  

  program_FLASH();

#ifdef VERBOSE  
  printf("PROGRAMMING DONE\n");
#endif

  //LOCK EEPROM AND FLASH
  lock_MASS_PROGRAM();

#ifdef VERBOSE
  printf("REBOOTING\n");
#endif 

  reboot_STM8L();

  return 0;
}