// During day-time the intensity of street light is varied using LDR
//At night during peak hours(23hr to 07hr) the light is controlled using PIR sensor.When the PIR sensor detects any motion it switches on the light for a specific period of time
//Here just for demonstration purpose the led will light up for 2 seconds only. LED represents the street light.
//The LCD will display time only during peak hours(23hr to 07hr) at night.
//To check this circuit during day-time, set hr value of function rtc_set_t(hr,mm,ss) between 06 to 23 [means 7am to 11pm] (24 hrs format), which is there in the main function.
//To check this circuit during night, set hr value of function rtc_set_t(hr,mm,ss) between 22 to 07 [means 11pm to 7am] (24 hrs format)  which is there in the main function.
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#define lcd PORTB

void serial_ini()
   {
       UCSRB=(1<<TXEN);
       UCSRC=(1<<UCSZ1)|(1<<UCSZ0)|(1<<URSEL);
       UBRRL=0X33;
    }
    
void serial_tr(unsigned char x)
   {
       while(!(UCSRA & (1<<UDRE)));
       UDR= x; 
       while(TXC ==0);
   }
 
void serial_tr_bcd(unsigned char x)
   {
       serial_tr('0'+(x>>4));
       serial_tr('0'+(x & 0x0f));
   }
 
void i2c_ini()
   {
       TWSR=0X00;
       TWBR=0X47;
       TWCR=0X04;
   }
 
void i2c_start()
  {
      TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
      while((TWCR &(1<<TWINT))==0);
  }
 
void i2c_wr(unsigned char x)
  {
     TWDR=x;
     TWCR=(1<<TWINT)|(1<<TWEN);
     while((TWCR & (1<<TWINT))==0);
 
  }
 
unsigned char i2c_re(unsigned char x)
   {
       TWCR=(1<<TWINT)|(1<<TWEN)|(x<<TWEA);
       while((TWCR &(1<<TWINT))==0);
       return TWDR;
   }
 
void i2c_stop()
   {
       TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	   int i;
       for(i=0;i<200;i++);
   }
 
void rtc_ini()
   {
      i2c_ini();
      i2c_start();
      i2c_wr(0xd0);   // address DS1307 for write
      i2c_wr(0x07);  //set register pointer to 7
      i2c_wr(0x00);  //set value of location 7 to 0
      i2c_stop();  // transmit stop condition
   }
 
void rtc_set_t(unsigned char h,unsigned char m,unsigned char s)
   {
       i2c_start();
       i2c_wr(0xd0);     // address DS1307 for write
       i2c_wr(0);        //set register pointer to 0
       i2c_wr(s);
       i2c_wr(m);
       i2c_wr(h);
       i2c_stop();
   }

void rtc_get_t(unsigned char *h,unsigned char *m,unsigned char *s)
   {
       i2c_start();
       i2c_wr(0xd0);     // address DS1307 for write
       i2c_wr(0);        //set register pointer to 0
       i2c_stop();
 
       i2c_start();
       i2c_wr(0xd1);     // address DS1307 for read
       *s=i2c_re(1);     //read sec ,read ack
       *m=i2c_re(1);     //read min ,read ack
       *h=i2c_re(0);     //read hour ,read nack
       i2c_stop();
   }

   void cmd(unsigned char x)
   {
       lcd=x;
       PORTD=(0<<3);
       PORTD=(0<<4);
       PORTD=(1<<5);
       _delay_ms(10);
       PORTD=(0<<5);
   }
 
void lcd_display(unsigned char x)
   {
      lcd=x;
      PORTD=(0<<4)|(1<<3);
      PORTD=(1<<5)|(0<<4)|(1<<3);
      _delay_ms(20);
      PORTD=(0<<5)|(0<<4)|(1<<3);
   }
 
void lcd_ini()
   {
       cmd(0x38);
       cmd(0x0e);
       cmd(0x01);
       cmd(0x06);
       cmd(0x80);
   }
 
void lcd_str(unsigned char *str)
   {
        while(*str!='\0')
          {
               lcd_display(*str);
               str++;
           }
    }
 
void lcd_pos(int line,int pos)
    {
        if(line==1)
            cmd(0x80+pos);
        else if(line==2)
            cmd(0xc0+pos);
    }
    
 void ADC_Init()
   {
       DDRA |=0x00;               /* Make ADC port as input */
       ADCSRA = 0x87;          /* Enable ADC, fr/128  */
       ADMUX = 0x40;           /* Vref: Avcc, ADC channel: 0 */
   }

int ADC_Read(char channel)
   {
	int Ain,AinLow;
	ADMUX=ADMUX|(channel & 0x0f);  /* Set input channel to read */
	ADCSRA |= (1<<ADSC);           /* Start conversion */
	while((ADCSRA&(1<<ADIF))==0);  /* Monitor end of conversion interrupt */
	_delay_us(10);
	AinLow = (int)ADCL;                /* Read lower byte*/
	Ain = (int)ADCH*256;           /* Read higher 2 bits and 
				       Multiply with weight */
       Ain = Ain + AinLow;             
       return(Ain);                    /* Return digital value*/
   }

int main()
 { 
    DDRD|=(1<<3) | (1<<4) | (1<<5) | (1<<7);
    DDRD |= (1<<2);
   DDRB=0XFF; 
    ADC_Init();
    MCUCR = 0x03;
   PORTD |= (0<<2);
    unsigned char i,j,k;
    int temp;
    rtc_ini();
       rtc_set_t(0x07,0x59,0x55);
        lcd_ini();
       serial_ini();
       
   while (1)
   {
      rtc_get_t(&i,&j,&k);
	       while((i==0x23) || (i<=6 && i>=0))
		  {
		     TCCR2 = 0x00;
		     sei();
		     GICR |= (1<<INT0);
		  /*if((PIND&(1<<2)))
		  {
		     cmd(0x0E);
		     //cmd(0x80);
		     TCCR2 = 0x69;
		     OCR2 = 255;
		     //GICR |= (1<<INT0);
		      lcd_str("TIME ");
		      lcd_pos(1,6);
		       lcd_display('0'+(i>>4));
		       lcd_display('0'+(i & 0x0f));
		       lcd_display(':');
		       lcd_display('0'+(j>>4));
		       lcd_display('0'+(j & 0x0f));
		       lcd_display(':');
		       lcd_display('0'+(k>>4));
		       lcd_display('0'+(k & 0x0f));
			lcd_pos(2,0);
			 lcd_str("Lights On");
			 _delay_ms(2000);
			 TCCR2 = 0x00;
			 cmd(0x01);
			cmd(0x08);  
		  }
		  */
		  rtc_get_t(&i,&j,&k);
	       }

       //rtc_get_t(&i,&j,&k);
	 cli();
	 cmd(0x08);
	    temp = ADC_Read(0);
	    if(temp< 100)
	    {
	       TCCR2 = 0x00;
	    }
	     else if(temp>=100 && temp<=230)
		       {
			  TCCR2 = 0x69;
			  OCR2 = 35;
			  }
		   
		       else if(temp>=231 && temp<=360)
		       {
			  TCCR2 = 0x69;
			  OCR2 = 70;
			  }
		   
		       else if(temp>=361 && temp<=490)
		       {
			  TCCR2 = 0x69;
			  OCR2 = 105;
			  }
			  
		       else if(temp>=491 && temp<=620)
		       {
			  TCCR2 = 0x69;
			  OCR2 = 140;
			  }
			  
		       else if(temp>=621 && temp<=750)
		       {
			  TCCR2 = 0x69;
			  OCR2 = 175;
			  }
			  
		       else if(temp>=751 && temp<=880)
		       {
			  TCCR2 = 0x69;
			  OCR2 = 210;
			  }
			  
			  else if(temp>=881 && temp<=1024)
		       {
			  TCCR2 = 0x69;
			  OCR2 = 255;
			  }
	if((i==0x23) || (i<=6 && i>=0))
	   TCCR2 = 0x00;
      }
   return 0;
 }
  ISR(INT0_vect)
    {
        unsigned char i =0,j = 0,k = 0,l,m,n;
        rtc_get_t(&i,&j,&k);
        l = i;
	m = j;
       n = k;
        cmd(0x0E);
		     //cmd(0x80);
		     TCCR2 = 0x69;
		     OCR2 = 255;
		     //GICR |= (1<<INT0);
		      lcd_str("TIME ");
		      lcd_pos(1,6);
		       lcd_display('0'+(l>>4));
		       lcd_display('0'+(l & 0x0f));
		       lcd_display(':');
		       lcd_display('0'+(m>>4));
		       lcd_display('0'+(m& 0x0f));
		       lcd_display(':');
		       lcd_display('0'+(n>>4));
		       lcd_display('0'+(n & 0x0f));
			lcd_pos(2,0);
			 lcd_str("Lights On");
			 _delay_ms(2000);
			 TCCR2 = 0x00;
			 cmd(0x01);
			cmd(0x08);  
    }
