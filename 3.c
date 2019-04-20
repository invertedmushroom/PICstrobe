/*Vkljuèi nastavitve za PIC16Fxxx*/
#include	<pic.h>
/*Vkljuèi nastavitve za LCD prikazovalnik*/
#include	"lcd.h"
/*Vkljuèi sprintf funkcijo*/
#include	<stdio.h>
#include	<stdlib.h>
/*definiraj frekvenca prikljuèenega oscilatorja!*/
#define _XTAL_FREQ 4000000
/*frekvenca notranje ure*/ 
#define _TAKT (_XTAL_FREQ/4) 
/*št. prekinitev za 1ms((notranja ura/št navodil za 1 prekinitev)*1ms) */
#define _SI_MS (_TAKT/200000) 

/*
 *	Prikljuèki na PICDEM 2 plus
 *
 *	Port A
 *  0	Potenciometer R16
 *  1	LCD
 *  2	LCD
 *  3	LCD
 *  4	Gumb S2(RA4)	
 *  5	D_1
 *  6
 *  7
 *
 *	Port B
 *	0	Gumb S3(RB0)/!LED1
 *	1	LED2 
 *	2	LED3
 *	3	LED4
 *	4	
 *	5	
 *	6	ICD	
 *	7	ICD
 *
 *	Port C
 *  0	
 *  1
 *  2	Buzzer
 *  3	EEPROM/Temp. senzor
 *  4	EEPROM/Temp senzor	
 *  5
 *  6	RS-232
 *  7	RS-232
 *
 *	Port D
 *  0	LCD
 *  1	LCD
 *  2	LCD
 *  3	LCD
 *  4	D_2	
 *  5	S_Desno
 *  6	S_Sredina
 *  7	S_Levo

 */
/*Tu so imenovani prikljuèki na èipu*/
#define	PORTBIT(adr, bit)	((unsigned)(&adr)*8+(bit))
static bit	gor  @	PORTBIT(PORTD, 4);
static bit	k_d  @	PORTBIT(PORTD, 5);
static bit	enter  @	PORTBIT(PORTD, 6);
static bit	dol  @	PORTBIT(PORTD, 7);
static bit	k_g  @	PORTBIT(PORTA, 5);

/*Tu so imenovane globalne spremenljivke*/
unsigned long st_i;        //število prekinitev
unsigned long r_i;         //izraèunano št. prekinitev
int obrati;
int frekvenca;
unsigned int poli;
unsigned int s_led_on; 	  //delay za led
unsigned int korak; 		  //izbran korak
unsigned int k_gd; 		  //izbira koraka
unsigned int p_meni_lcd;   //pozicija v meniju
unsigned char ven_p[17];	  //niz 16ih znakov za prvo vrstico na LCD. +1 za zakljuèitev niza
unsigned char ven_d[17];	  //druga vrstica
char slip_lcd[6];
unsigned int meni;

void gordol(unsigned int kam);
void izberi_korak(void);
void prikazi(unsigned int i_meni);
void cakaj(unsigned int caki);
void izracun(void);

/*Prekinitvena funkcija, edina prekinitev je T0IF, zato ni potrebno preverjati katera prekinitev je.
Prekinitev se zgodi vsakih 200 navodil oziroma, ko TMR0 doseže vrednost 255.*/
void interrupt
timer(void)
{
/*Ob vsaki prekinitvi je T0IF=1.Za nadalnje proženje prekinitev moramo postaviti T0IF na 0*/
	T0IF = 0; //ponastavi zastavico, da se je zgodila prekinitev
	++st_i; //Število prekinitev(st_i) se poveèa za 1
	TMR0 = 55; //TMR overflow vsakih 200 navodil

/*Èe je število prekinitev enako izraèunanemu številu prekinitev se prižge LED dioda za 1.4ms.
Èas ko je LED prižgana doloèa spodnja funkcija*/
	if (st_i >= r_i)
	 {
	 PORTB |= 0x0f; //Prižgi LED

/*zakasnitvena funkcija 20x izvede ukaz "nop", kateri ne izvede nobene funkcije, ampak samo porabi èas*/
	 do {
	 asm("nop");
	 asm("nop");
	 ++s_led_on;
	 }while (s_led_on <= 10);
	 s_led_on = 0;
	 
	 PORTB &= 0x00; //Ugasni LED
	 st_i = 0; //Ponastavi za zaèetek nove periode 
	 }
}
void izracun(void)
{
/*(((1/zeljena frekvenca)*1000(izracunaj v ms)*(stevilo prekinitev za 1ms))*60(ker raèuna z obrati))
-(èas ko je LED prižgana: 1.4ms/200us= 7 prekinitev)*/
r_i = ((1000* _SI_MS*60)/obrati)-7;
}
/*Zakasnitvena funkcija, uporabljena veèkrat v programu*/
void cakaj(unsigned int caki)
{
unsigned int caki_p;
caki_p = 0;
do {
__delay_ms(50);
++caki_p;
}while (caki_p <= caki);
}

/*Funkcija za prikaz izbora na LCD zaslonu*/
void prikazi(unsigned int i_meni)
{
if(i_meni == 1) //glavni meni
 {
 lcd_clear();
/*switch izbere 1,2,3 ali 4 opcijo, odvisno od položaja preklopnika*/
 switch(p_meni_lcd)
  {
/*sprintf funkcija zapiše besedilo v narekovajih v niz znakov, berljivih LCD zaslonu*/
  case 1: {sprintf(ven_p, "merjenje"); sprintf(ven_d, "obratov rotorja");} break;
  case 2: {sprintf(ven_p, "merjenje"); sprintf(ven_d, "obratov in slipa");} break;
  case 3: {sprintf(ven_p, "merjenje"); sprintf(ven_d, "zunanje fr.");} break;
  case 4: {sprintf(ven_p, "zunanje"); sprintf(ven_d, "prozenje");} break;
  }
 }

if(i_meni == 2) //meni merjenja frekvence
 {
 lcd_clear();
/*sprintf funkcija zapiše besedilo v narekovajih in spremenljivko v niz znakov, berljivih LCD zaslonu*/
 sprintf(ven_p, "%d ob/min", obrati);
 sprintf(ven_d, "korak %d", korak);
 }

if(i_meni == 3) //meni omrežne frekvence
 {
 lcd_clear();
 sprintf(ven_p, "fr. statorja je");
 sprintf(ven_d, "%d Hz korak %d", frekvenca, korak);
 }

if(i_meni == 4) //št polov
 {
 lcd_clear();
 sprintf(ven_p, "st. polov je %d", poli);
 sprintf(ven_d, "obrati: %d", obrati);
 }

if (i_meni==5) //meni prikaza izraèuna
 {
 lcd_clear();
 sprintf(ven_p, "%d o/min", obrati);
 sprintf(ven_d, "slip: %s", slip_lcd);
 }

/*zapiše izbran meni na LCD*/
 lcd_goto(0);		//Izbere 1 vrstico na LCD
 lcd_puts(ven_p);
 lcd_goto(0x40);	//Izbere 2 vrstico na LCD
 lcd_puts(ven_d);

}
void izberi_korak(void)
{
switch (k_gd)
 {
 case 1: korak = 1; break;
 case 2: korak = 10; break;
 case 3: korak = 100; break;
 case 4: korak = 1000; break;
 }
}
/*funkcija, ki preverja spremembo na tipkah za korak in enter, ter na kodirniku*/
void gordol(unsigned int kam)
{
if(!k_g) //povecaj korak
 {
 cakaj(1);
 ++k_gd;
 if(k_gd >> 4) k_gd = 4;
 izberi_korak();
 if(kam==2) prikazi(2);
 if(kam==3) prikazi(3);
 }

if(!k_d) //zmanšaj korak
 {
 cakaj(1);
 --k_gd;
 if(k_gd == 0) k_gd = 1;
 izberi_korak();
 if(kam==2) prikazi(2);
 if(kam==3) prikazi(3);
 }

/*Èe je kodirnik zavrten v desno*/
if(!gor)
	{
	cakaj(1);
	switch (kam)
		{
		case 1: {++p_meni_lcd; if(p_meni_lcd >= 4) p_meni_lcd = 4; prikazi(1);} break; //v glavni meni
		case 2: {obrati += korak; {if(obrati >= 30000)obrati=30000;} izracun(); prikazi(2);} break; //poveèaj obrate za korak
		case 3: {frekvenca += korak; {if(frekvenca >= 500)frekvenca = 500;} obrati = frekvenca*60; izracun(); prikazi(3);} break;
		case 4: {++poli; {if(poli >= 20)poli = 20;} obrati = (frekvenca*60)/poli; izracun(); prikazi(4);} break;
		}
	}

/*Èe je kodirnik zavrten v levo*/
if(!dol)
	{
	cakaj(1);
	switch (kam)
		{
		case 1: {--p_meni_lcd; if(p_meni_lcd <= 0) p_meni_lcd = 1; prikazi(1);} break; //v glavni meni
		case 2: {obrati -= korak; {if(obrati <= 1) obrati = 1;} izracun(); prikazi(2);} break; //zmanjšaj obrate za korak
		case 3: {frekvenca -= korak; {if(frekvenca <= 1)frekvenca = 1;} obrati = frekvenca*60; izracun(); prikazi(3);} break;
		case 4: {--poli; {if(poli <= 1)poli = 1;} obrati = (frekvenca*60)/poli; izracun(); prikazi(4);} break;
		}
	}
}
/*Glavna funkcija, tu se program zaène*/
main()
{
 /*Zaèetek neskonène zanke*/
for(;;)
	{
//Tukaj imenujemo spremenljivke, katere uporabljamo v funkciji main()
	unsigned int izhod;
	unsigned int n_obrati;
	int slip_i;

/*Izkljuèi primerjalnike in PWM(dokumentacija o PIC16F877 str63)*/
	CCP1CON &= 0x00;
	CCP2CON &= 0x00;
		
/*Izkljuèi analogne vhode in jih naredi digitalne(str127)*/
	ADCON0 &= 0x00;
	ADCON1 &= 0x06;
		
/*uporabi nezdeljen TMR0(255 navodil);OPTION_REG = RBPU(bit 7)(1=DISABLED)(str44), 
T0CS(bit 5)(1=T0CKI, 0=notranja),	PSA(bit 3)(1=WDT, 0=TMR0);0b00001000(str23)*/
	OPTION &= 0x88;
	
/* RA5 (1b)vhod ostalo so (0b)izhodi(nerabljeni prikljuèki so nastavljeni na izhod, ker tako zmanjšamo porabo toka)(str44)*/
	TRISA = 0x10;	
	TRISB = 0x00;			
	TRISC = 0x00;
	TRISD = 0xf0;
	
/*LCD funkcija, ki doloèi vhodno in izhodne prikljuèke za LCD, pobriše zaslon ter se postavi v prvo vrstico*/
	lcd_init();

/*doloèi zaèetne vrednosti spremenljivk*/
	obrati = 3000;
	korak = 1;
	p_meni_lcd = 0;
	frekvenca = 50;

/*Izpiše "stroboskopski merilec obratov", ter prižge luè za 1/2s*/
	lcd_goto(0);	//Izbere 1 vrstico na LCD
	lcd_puts("stroboskopski");
	lcd_goto(0x40);	//Izbere 2 vrstico na LCD
	lcd_puts("merilec obratov");
	PORTB = 0x0f;	//posveti za 1/2s
/*zakasnitev bi se zaradi prekinitev, poveèala za 1.4 ms vsakiè, ko je LED prižgana(veèji so obrati, veèja je zakasnitev)*/
	cakaj(10);
	PORTB = 0x00;
	
/*vkljuèi GIE in TMR0(str54) overflow interrupt(str24)10100000*/
	INTCON = 0xA0;
	for(;;) 
        {
/*Tu se program zares zaène, na LCD piše "stroboskopski merilec obratov", dokler ne pritisnemo tipke enter.
Ob pritisku se izpiše "glavni meni", po glavnem meniju pa se lahko pomikamo s tipko gor ali dol. Izbor funkcije potrdimo s tipko enter.*/
		if(!enter)
		{
		lcd_clear(); lcd_goto(0); lcd_puts("glavni meni");
		cakaj(3); 

/*Dokler je izhod=1, se izvaja funkcija, zapisana v zavitih oklepajih. Tipka enter zapiše 0 v izhod, program skoèi iz while zanke ter zapiše "OK" na LCD*/
		izhod = 1;
		 while(izhod == 1)
		 {
/*Funkcija preverja pritisk na tipki gor in dol. Ob pritisku spreminja spremenljivko "p_meni_lcd"*/
		 gordol(1);
		 if(!enter) {izhod = 0; lcd_clear(); lcd_goto(0); lcd_puts("OK!");}
		 }
		}

/*Èe smo izbrali 1. opcijo v meniju se program nadaljuje tukaj, drugaèe preskoèi na prevejanje 2. opcije*/
		if(p_meni_lcd == 1)
		 {
		 cakaj(2);
		 lcd_clear();
		 prikazi(1);
		 izhod = 1;
		  while(izhod == 1)
		  {
		  gordol(2);	//sprememba obratov, koraka in izraèun
		  if(!enter) {izhod = 0; lcd_clear(); p_meni_lcd = 0;}
		  }
		 }

		if(p_meni_lcd == 2)
		 {
		 cakaj(5);
		 prikazi(1);

		 izhod = 1;
		  while(izhod == 1) //
		  {
		  gordol(3); //nastavi frekvenco omrežja in korak
		  if(!enter) {izhod = 0; lcd_clear();}
		  }

		 izhod = 1;
		 lcd_goto(0);
		 lcd_puts("vpisi stevilo");
		 lcd_goto(0x40);
		 lcd_puts("polovih parov");
		 cakaj(5);
		  while(izhod == 1)
		  {
		  gordol(4); //vpisi pole in utripaj po nazivnih obratih za št. polov
		  if(!enter) {izhod = 0; lcd_clear();}
		  }

		 izhod = 1;
		 lcd_goto(0);
		 lcd_puts("nastavi obrate");
		 lcd_goto(0x40);
		 lcd_puts("rotorja");
		 cakaj(5);
		  while(izhod == 1)
		  {
		  gordol(2);	//nastavi realne obrate
		  if(!enter) {izhod = 0; lcd_clear();}
		  }
		 izhod = 1;
		 cakaj(5);
		 lcd_clear();
		 /*izraèunaj slip iz nazivnih in realnih obratov*/
		 n_obrati = (frekvenca/poli)*60; //izraèunaj nazivne obrate
/*Izraèuna slip in pomakne decimalno vejico za 3 mesta v desno, decimalen del zanemari, ker je delo z decimalnimi števili
zamudno in porabi veliko procesorskega pomnilnika*/
		 slip_i = (((n_obrati-obrati)*1000)/n_obrati);

/*Zapiši slip v niz znakov. Niz je sestavljen iz 0.xxx , kjer je xxx izraèunan slip*/
			slip_lcd[0] = 48; // 0
			slip_lcd[1] = 46; //.
			slip_lcd[2] = ((slip_i/100)%10)+48; //1. decimalno mesto slipa
			slip_lcd[3] = ((slip_i/10)%10)+48; //2 dec. mesto
			slip_lcd[4] = (slip_i%10)+48; //3.dec. mesto
			slip_lcd[5] = 0; //zakljuèi niz

		  while(izhod == 1)
		  {
/*Prikaže obrate in izraèunan slip na LCD*/
			prikazi(5);
		  	if(!enter) {izhod = 0; lcd_clear(); p_meni_lcd = 0;}
		  }

		 }		 	
	
		} //for(;;)
	} //for(;;) init
} //main