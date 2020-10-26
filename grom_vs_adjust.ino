#define SCALE	1.0

typedef unsigned char	UCHAR, BOOL;
typedef unsigned		UINT;
typedef unsigned long	ULONG;

//#define DEBUG

//////////////////////////////////////////////////////////////////////////////
// Timer1 int handler (input capture)

#define OutPulseEnable()	( TCNT3 = 0xFFFF, TCCR3B |= 3 )		// prescaler: 1/64
#define OutPulseDisable()	( TCCR3B &= ~0x7 )	// stop timer

volatile UINT g_uInPulseWidth;
volatile UCHAR g_uState = 0;

ISR( TIMER1_CAPT_vect ){
	
	// パルス幅を保存する
	
	static UINT uPrevCnt = 0;
	UINT uCnt = ICR1;
	g_uInPulseWidth = uCnt - uPrevCnt;
	uPrevCnt = uCnt;
	
	TCNT4 = 0; // reset sv wdt
	
	if( g_uState < 2 ){
		if( g_uState == 1 ){
			// 2パルス入力されたので，パルス出力可能になる
			OutPulseEnable();
		}
		++g_uState;
	}
}

//////////////////////////////////////////////////////////////////////////////
// Timer3 overflow int handler (output vs pulse)

ISR( TIMER3_OVF_vect ){
	UINT uPulseWidth = g_uInPulseWidth;
	UINT uPulseWidthAdjust;
	
	interrupts();
	
	if(
		SCALE < 1.0 &&
		uPulseWidth >= ( UINT )( 0xFFFFUL * SCALE )
	){
		// 除算後 0xFFFF を超えそうな場合
		uPulseWidthAdjust = 0xFFFF;
	}else{
		uPulseWidthAdjust = (( ULONG )uPulseWidth * (( ULONG )( 0x10000 / SCALE ))) >> 16;
	}
	
	OCR3A = uPulseWidthAdjust >> 1;
	ICR3  = uPulseWidthAdjust - 1;
	
	static UCHAR uLed = 0;
	if( uLed ^= 1 ){ RXLED1; }else{ RXLED0; }
}

//////////////////////////////////////////////////////////////////////////////
// Timer4 overflow int handler

ISR( TIMER4_OVF_vect ){
	interrupts();
	
	// ovf 発生は 0.3km/h 未満を意味するので，パルス出力を停止する
	OutPulseDisable();
	g_uState = 0;
}

//////////////////////////////////////////////////////////////////////////////
// setup

void setup(){
	//////////////////////////////////////////////////////////////////////////
	// setup Timer3 (PWM output)
	
	pinMode( 5, OUTPUT );	// OC3A/PC6
	
	TIMSK3 = 1;	// ovf int enable
	
	#define WGM3 0xE	// fast pwm, top = ICRn
	
	TCCR3A =
		( 2 << 6 ) |	// COMnA: output low when match
		( WGM3 & 0x3 );	// WGM3[1:0]
	
	TCCR3B =
		(( WGM3 >> 2 ) << 3 );	// WGM3[3:2]
	
	//////////////////////////////////////////////////////////////////////////
	// setup Timer1 (capture)
	
	pinMode( 4, INPUT_PULLUP );	// ICP1/PD4
	
	TIMSK1 = ( 1 << 5 );		// capture int
	
	#define WGM1 0x4			// CTC top:OCRnA
	
	TCCR1A = ( WGM1 & 0x3 );	// WGM1[1:0]
	
	OCR1A = 0xFFFF;				// top value ★TCCR1A より前に書くとなぜか破壊される
	TCCR1B =
		( 1 << 7 ) |			// noise canceler
		( 1 << 6 ) |			// capture posedge
		(( WGM1 >> 2 ) << 3 ) |	// WGM1[3:2]
		3;						// prescaler: 1/64
	
	//////////////////////////////////////////////////////////////////////////
	// setup Timer4 (vs pulse timeout)
	
	TCCR4A = 0;
	TCCR4C = 0;
	TCCR4D = 0;
	TCCR4E = 0;
	DT4    = 0;
	
	OCR4C = 0xEF;			// ovf cnt
	TIMSK4 = ( 1 << 2 );	// ovf int
	
	TCCR4B = 0xF;			// CK/16384
	
	//////////////////////////////////////////////////////////////////////////
	// for debug
	
	pinMode( 8, OUTPUT );
	pinMode( 9, OUTPUT );
}

//////////////////////////////////////////////////////////////////////////////

void loop(){
	delay( 50 );
	#ifdef DEBUG
		Serial.println( g_uState >= 2 ? g_uInPulseWidth : 0 );
	#endif
	
	static char cLed = 0; digitalWrite( 8, cLed ^= 1 );
}
