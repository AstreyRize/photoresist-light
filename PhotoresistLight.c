/*
 * PhotoresistLight.c
 *
 * Created: 28.03.2018 21:08:26
 * Author: Astrey
 */

#include <mega8.h>
#include <delay.h>

// Таймер запущен.
unsigned char timerIsStart = 0;

// Номер выделенного разряда.
unsigned char digitNumber = 3;

// Таймер должен переполниться 30 раз, что бы прошла секунда.
unsigned char OverflowsRemain = 30;

// Цифры для kem-5461ar
unsigned char numbers[11] =
{
	//PB7...PB0
	//FBGCDpDEA
    0b11010111, //0
    0b01010000, //1
    0b01100111, //2
    0b01110101, //3
    0b11110000, //4
    0b10110101, //5
    0b10110111, //6
    0b01010001, //7
    0b11110111, //8
    0b11110101, //9
    0b00100000  //-
};

// Точка.
unsigned char dot = 0b00001000;

// Разряды.
unsigned char digit[4] =
{
    0b11111101, // 1 разряд слева.
    0b11111011, // 2 разряд слева.
    0b11110111, // 3 разряд слева.
    0b11111110  // 4 разряд слева.
};

unsigned char digitByNumbers[4] =
{
    0, // 1 разряд слева.
    0, // 2 разряд слева.
    2, // 3 разряд слева.
    3  // 4 разряд слева.
};

// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
    // Reinitialize Timer 0 value
    TCNT0 = 0;

    OverflowsRemain--;

    if(OverflowsRemain == 0)
    {
        OverflowsRemain = 30;

        if(digitByNumbers[3] > 0)
        {
            digitByNumbers[3]--;
            return;
        }
        if(digitByNumbers[2] > 0)
        {
            digitByNumbers[2]--;
            digitByNumbers[3] = 9;
            return;
        }
        if(digitByNumbers[1] > 0)
        {
            digitByNumbers[1]--;
            digitByNumbers[2] = 9;
            digitByNumbers[3] = 9;
            return;
        }
        if(digitByNumbers[0] > 0)
        {
            digitByNumbers[0]--;
            digitByNumbers[1] = 9;
            digitByNumbers[2] = 9;
            digitByNumbers[3] = 9;
            return;
        }

        // Тут таймер досчитал до нуля.
        PORTD.1 = 0;

        // Выключаем таймер.
        TCCR0=(0<<CS02) | (0<<CS01) | (0<<CS00);

        //Включаем прерывания энкодера
        GICR |= (1<<INT1) | (1<<INT0);

        // Предустановленное значение: 0
        TCNT0 = 0;

        timerIsStart = 0;
    }
}

// External Interrupt 0 service routine
interrupt [EXT_INT0] void ext_int0_isr(void)
{
    // Считываем значения порта D4 и если уровень высокий,
    // отнимаем единицу, если незкий, прибавляем единицу.
    if(PIND.4)
    {
        if(digitByNumbers[digitNumber] < 9)
        {
            digitByNumbers[digitNumber]++;
        }
    }
    else
    {
        if(digitByNumbers[digitNumber] > 0)
        {
            digitByNumbers[digitNumber]--;
        }
    }
}

// External Interrupt 1 service routine
interrupt [EXT_INT1] void ext_int1_isr(void)
{
    if(digitNumber == 0)
    {
        digitNumber = 3;
    }
    else
    {
        digitNumber--;
    }
}

void main(void)
{
// Input/Output Ports initialization
// Port B initialization
// Function: Bit7=Out Bit6=Out Bit5=Out Bit4=Out Bit3=Out Bit2=Out Bit1=Out Bit0=Out
DDRB=(1<<DDB7) | (1<<DDB6) | (1<<DDB5) | (1<<DDB4) | (1<<DDB3) | (1<<DDB2) | (1<<DDB1) | (1<<DDB0);
// State: Bit7=1 Bit6=1 Bit5=1 Bit4=1 Bit3=1 Bit2=1 Bit1=1 Bit0=1
PORTB=(1<<PORTB7) | (1<<PORTB6) | (1<<PORTB5) | (1<<PORTB4) | (1<<PORTB3) | (1<<PORTB2) | (1<<PORTB1) | (1<<PORTB0);

// Port C initialization
// Function: Bit6=In Bit5=In Bit4=In Bit3=Out Bit2=Out Bit1=Out Bit0=Out
DDRC=(0<<DDC6) | (0<<DDC5) | (0<<DDC4) | (1<<DDC3) | (1<<DDC2) | (1<<DDC1) | (1<<DDC0);
// State: Bit6=T Bit5=T Bit4=T Bit3=1 Bit2=1 Bit1=1 Bit0=1
PORTC=(0<<PORTC6) | (0<<PORTC5) | (0<<PORTC4) | (1<<PORTC3) | (1<<PORTC2) | (1<<PORTC1) | (1<<PORTC0);

// Port D initialization
// Function: Bit7=In Bit6=In Bit5=In Bit4=In Bit3=In Bit2=In Bit1=In Bit0=In
DDRD=(0<<DDD7) | (0<<DDD6) | (1<<DDD5) | (1<<DDD4) | (1<<DDD3) | (1<<DDD2) | (1<<DDD1) | (0<<DDD0);
// State: Bit7=T Bit6=T Bit5=T Bit4=T Bit3=T Bit2=T Bit1=T Bit0=T
PORTD=(0<<PORTD7) | (0<<PORTD6) | (0<<PORTD5) | (0<<PORTD4) | (0<<PORTD3) | (0<<PORTD2) | (0<<PORTD1) | (0<<PORTD0);

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: Timer 0 Stopped
TCCR0=(0<<CS02) | (0<<CS01) | (0<<CS00);
TCNT0=0x00;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: Timer1 Stopped
// Mode: Normal top=0xFFFF
// OC1A output: Disconnected
// OC1B output: Disconnected
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (0<<CS12) | (0<<CS11) | (0<<CS10);
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x00;
OCR1AL=0x00;
OCR1BH=0x00;
OCR1BL=0x00;

// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: Timer2 Stopped
// Mode: Normal top=0xFF
// OC2 output: Disconnected
ASSR=0<<AS2;
TCCR2=(0<<PWM2) | (0<<COM21) | (0<<COM20) | (0<<CTC2) | (0<<CS22) | (0<<CS21) | (0<<CS20);
TCNT2=0x00;
OCR2=0x00;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=(0<<OCIE2) | (0<<TOIE2) | (0<<TICIE1) | (0<<OCIE1A) | (0<<OCIE1B) | (0<<TOIE1) | (0<<TOIE0);

// External Interrupt(s) initialization
// INT0: On
// INT0 Mode: Rising Edge
// INT1: On
// INT1 Mode: Falling Edge
GICR|=(1<<INT1) | (1<<INT0);
MCUCR=(1<<ISC11) | (0<<ISC10) | (1<<ISC01) | (1<<ISC00);
GIFR=(1<<INTF1) | (1<<INTF0);

// USART initialization
// USART disabled
UCSRB=(0<<RXCIE) | (0<<TXCIE) | (0<<UDRIE) | (0<<RXEN) | (0<<TXEN) | (0<<UCSZ2) | (0<<RXB8) | (0<<TXB8);

// Analog Comparator initialization
// Analog Comparator: Off
// The Analog Comparator's positive input is
// connected to the AIN0 pin
// The Analog Comparator's negative input is
// connected to the AIN1 pin
ACSR=(1<<ACD) | (0<<ACBG) | (0<<ACO) | (0<<ACI) | (0<<ACIE) | (0<<ACIC) | (0<<ACIS1) | (0<<ACIS0);
SFIOR=(0<<ACME);

// ADC initialization
// ADC disabled
ADCSRA=(0<<ADEN) | (0<<ADSC) | (0<<ADFR) | (0<<ADIF) | (0<<ADIE) | (0<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);

// SPI initialization
// SPI disabled
SPCR=(0<<SPIE) | (0<<SPE) | (0<<DORD) | (0<<MSTR) | (0<<CPOL) | (0<<CPHA) | (0<<SPR1) | (0<<SPR0);

// TWI initialization
// TWI disabled
TWCR=(0<<TWEA) | (0<<TWSTA) | (0<<TWSTO) | (0<<TWEN) | (0<<TWIE);

// Global enable interrupts
#asm("sei")

while (1)
      {
            unsigned char step = 0;

            // Если кнопка нажата.
            if(PIND.5 == 1  && timerIsStart == 0){
                timerIsStart = 1;

                //Выключаем прерывания энкодера
                GICR = (0<<INT1) | (0<<INT0);

                // Прерывание при переполнении: Вкл.
                TIMSK |= (1 << TOIE0);

                // Предустановленное значение: 0
                TCNT0 = 0;

                // Делитель: Clk/1024
                TCCR0 = (1<<CS02) | (0<<CS01) | (1<<CS00);

                PORTD.1 = 1;
            }

            do{
                PORTC = digit[step];

                if(step == digitNumber)
                {
                    PORTB = numbers[digitByNumbers[step]] | dot;
                }
                else
                {
                   PORTB = numbers[digitByNumbers[step]];
                }

                delay_ms(5);

                step++;
            }while(step < 4);
      }
}