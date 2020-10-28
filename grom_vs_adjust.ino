#define SCALE	0.905

typedef unsigned char	UCHAR, BOOL;
typedef unsigned		UINT;
typedef unsigned long	ULONG;

//#define DEBUG

//////////////////////////////////////////////////////////////////////////////
// Timer1 int handler (input capture)

volatile UINT g_uInPulseWidth;
#define STOP	0xFFFF
#define START	0xFFFE

ISR( TIMER1_CAPT_vect ){
	
	// パルス幅を保存する
	
	static UINT uPrevCnt = 0;
	UINT uCnt = ICR1;
	if( g_uInPulseWidth == STOP ){
		g_uInPulseWidth = START;
	}else{
		g_uInPulseWidth = uCnt - uPrevCnt;
	}
	uPrevCnt = uCnt;
	
	TCNT4 = 0; // reset sv wdt
}

//////////////////////////////////////////////////////////////////////////////
// Timer3 overflow int handler (output vs pulse)

#define OutPulseEnable()	( TCNT3 = 0xFFFF, TCCR3B |= 3 )		// prescaler: 1/64
#define OutPulseDisable()	( TCCR3B &= ~0x7 )	// stop timer
#define IsOutPulseEnabled()	( TCCR3B & 0x7 )

volatile UINT g_uOutPulseWidth;

ISR( TIMER3_OVF_vect ){
	UINT uPulseWidth = g_uOutPulseWidth;
	
	OCR3A = uPulseWidth >> 1;
	ICR3  = uPulseWidth - 1;
	
	static UCHAR uLed = 0;
	if( uLed ^= 1 ){ RXLED1; }else{ RXLED0; }
}

//////////////////////////////////////////////////////////////////////////////
// Timer4 overflow int handler

ISR( TIMER4_OVF_vect ){
	interrupts();
	
	// ovf 発生は 0.3km/h 未満を意味するので，パルス出力を停止する
	OutPulseDisable();
	g_uInPulseWidth = STOP;
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
	
	//pinMode( 8, OUTPUT );
	//pinMode( 9, OUTPUT );
}

//////////////////////////////////////////////////////////////////////////////

void loop(){
	
	static UINT		uPrevPulseWidth	= ~0;
	
	UINT	uPulseWidth;
	
	// ステート変更まち
	while( 1 ){
		noInterrupts();
		uPulseWidth	= g_uInPulseWidth;
		interrupts();
		
		if( uPulseWidth < START && uPulseWidth != uPrevPulseWidth ) break;
	}
	
	uPrevPulseWidth	= uPulseWidth;
	
	// 補正パルス幅の計算
	UINT uPulseWidthAdjust;
	
	if(
		SCALE < 1.0 &&
		uPulseWidth >= ( UINT )( 0xFFFFUL * SCALE )
	){
		// 除算後 0xFFFF を超えそうな場合
		uPulseWidthAdjust = 0xFFFF;
	}else{
		uPulseWidthAdjust = (( ULONG )uPulseWidth * (( ULONG )( 0x10000 / SCALE ))) >> 16;
	}
	
	noInterrupts();
	g_uOutPulseWidth = uPulseWidthAdjust;
	interrupts();
	
	// パルス出力再開
	if( !IsOutPulseEnabled()) OutPulseEnable();
}
