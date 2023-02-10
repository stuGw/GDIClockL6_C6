#include "rve.h"
#include "gdi.h"
#include "system.h"
#include "time.h"

//extern struct TimeNDate tmdt;
/*rve timer!!!!!!!!!!!!!
void (*rveEff[])(char ind,char val) = 
{
	rveSdvigRight,
	rveSdvigLeft,
	rveFreq,
	rveSetBright,
	rveDateNTime,
	rveAddCath,
	rveOffButComma,
	rveRue
};
//Таймер эффектов
void TIM1_UP_IRQHandler(void)
{
	static unsigned  int cntr = 0;

	static unsigned int radCounter = 0;
	static unsigned int radEffCounter = 0;
	static unsigned char svBright[6] = {70,70,70,70,70,70};
	static unsigned short int rveRueInterv = 100;
	char num,yark,ind,val;
	unsigned short int tm,interv;
	
	cntr++;
	radCounter++;
	
	if(flagRve == 0)
	{
		catFade[currGdiEff](5,cntr);
		catFade[currGdiEff](4,cntr);
		catFade[currGdiEff](3,cntr);
		catFade[currGdiEff](2,cntr);
		catFade[currGdiEff](1,cntr);
		catFade[currGdiEff](0,cntr);
	}
	//for PL2 GDIEFF
	if(secFlag)
	{
		cntr = 0;
		secFlag = 0;	
	}
	
	if(flagRveEff == 0)
	{
		flagRve = 0;
		TIM1->SR &= ~TIM_SR_UIF; //Clean UIF Flag
		return;
	}
	
	if(radCounter%rveInterval == 0)
	{	
		flagRve = RVE_EFF_RUE;
		
		rveRueInterv = rveTimeout/10;
		
		svBright[0] = gdi[0].brAll;
		svBright[1] = gdi[1].brAll;
		svBright[2] = gdi[2].brAll;
		svBright[3] = gdi[3].brAll;
		svBright[5] = gdi[5].brAll;
		svBright[4] = gdi[4].brAll;//[gdi[4].currCath];
		
		//rveSdvigRight(RVE_SDV_CURR,0);
	
		getCurrRveEff(&num,&ind,&yark,&val,&tm,&interv);
		if(num == 4)
		rveEff[num](ind,val);
		else if((num == 5)||(num == 7))
			rveEff[num](ind,yark);
		else 
			rveEff[num](ind,yark);
	//	rveFreq(300);
		//rveDateNTime();
		//rveAddCath(3,9);
		//rveOffButComma(4,0);
		//rveSetBright(4,5);
	//	rveSetBright(5,80);
     // rveOffButComma(1,10);
		 //rveOffButComma(2,10);
		 //rveOffButComma(3,10);
		 //rveOffButComma(4,10);
		 //rveOffButComma(5,10);
		  //rveOffButComma(0,10);
		//	rveSetBright(0,0);
		//rveSetBright(1,1);
	//	rveSetBright(2,2);
	//	rveSetBright(3,3);
	//	rveSetBright(4,4);
	//	rveSetBright(5,5);
	}
	
	if(flagRve>0)
		radEffCounter++;
	
	if(flagRve == RVE_EFF_RUE)
	{
		if((radEffCounter%rveRueInterv) == 0)
		{
		//	rveRue(0,0);
		//	rveRue(5,0);
		}
	}
	
	if(radEffCounter>rveTimeout)
	{
		
		
		setIndCath(0,tmdt.hour/10);
		setIndCath(1,tmdt.hour%10);
    setIndCath(2,tmdt.min/10);
		setIndCath(3,tmdt.min%10);
		setIndCath(4,tmdt.sec/10);
		setIndCath(5,tmdt.sec%10);
		rveFreq(10,10);
	//	rveRetValue();
		rveSetBright(0,svBright[0]);
		rveSetBright(1,svBright[1]);
		rveSetBright(2,svBright[2]);
		rveSetBright(3,svBright[3]);
		rveSetBright(4,svBright[4]);
		rveSetBright(5,svBright[5]);
		offIndComma(0);
		offIndComma(1);
		offIndComma(2);
		offIndComma(3);
		offIndComma(4);
		offIndComma(5);
		radEffCounter = 0;
		flagRve = 0;
	}
//	radCounter++;
	TIM1->SR &= ~TIM_SR_UIF; //Clean UIF Flag
}


char getRveRnd(char* effWeight,char size)
{
	 char i = 0;
   char weightSumm = 0;
   char number = 0;
   int num= 0;
   char n = 0;
   while(i<size)
       weightSumm+=effWeight[i++];
//   num = rand();
   number = num%weightSumm;
   i = 0;
   while(i<size)
   {
        n+=effWeight[i];
        if(n>number)return i;
        i++;
   }
	 return i;
}



char effArr[8] = {0,1,2,3,4,5,6,7};
 char effArrWeight[8] = {4,4,0,8,0,0,0,0};

char effIndArr[15] = {0,1,2,3,4,5,16,13,14,24,25,26,35,36,46};
char effIndWeight[15] = {4,3,3,3,3,3,2,1,2,1,2,1,1,1,1};

char effValArr[10] = {0,1,2,3,4,5,6,7,8,9};
char effValWeight[10] = {3,2,1,3,2,3,2,2,1,3};

char effBrightArr[18] = {0,1,2,3,4,6,8,10,20,25,27,30,35,40,45,50,55,59};
char effBrightWeight[18] = {4,4,4,3,3,3,2,2,1,1,1,1,1,1,1,2,3,1};

unsigned short int effTimeout[10] = {50,100,200,300,500,700,1000,1500,2000,2500};
 char effTimoutWeight[8][10] =
{
    {1,1,1,3,3,2,3,1,2,1},//SdvigLEFT
    {1,1,1,3,3,2,3,1,2,1},//SdvigRight
    {1,1,1,3,3,2,3,1,2,1},//RVE_Freq
    {1,1,1,2,2,1,1,1,1,1},//CHANGE_BRIGHT
    {1,1,1,3,3,2,3,1,2,1},//DATE_N_TIME
    {1,1,1,3,3,2,3,1,2,1},//ADDCATHODE
    {1,1,1,3,3,2,3,1,2,1},//OFF_B_COMMA
    {1,1,1,3,3,2,3,1,2,1}//RUE
};

//get effNum =  Eff Number getRveRnd(effArrWeight,8);
//get Ind Number getRveRnd(effIndWeight,15)
//get Br Value getRveRnd(effBrightWeight)
//get Val getRveRnd(effValWeight)

//get eff Timeout getRveRnd(effTimeoutWeight[effNum],10)
//get eff Interval getRveRnd(effInterval
void getCurrRveEff(char* numEff, char* ind, char* yark,char* val, unsigned short int* timeout, unsigned short int* interval)
{
	*numEff = effArr[getRveRnd(effArrWeight,8)];//Номер эффекта
	*ind = effIndArr[getRveRnd(effIndWeight,15)];//Индекс
	*yark = effBrightArr[getRveRnd(effBrightWeight,18)];
	*val = effValArr[getRveRnd(effValWeight,10)];
	
}



//Postapocalyptic rve effs
//Sdvig vsego znachenija
void rveSdvigRight(char sdvVal, char val)
{
	unsigned char tmp = 0;
	tmp = gdi[5].currC;
	setIndCath(5,gdi[4].currC);
	setIndCath(4,gdi[3].currC);
	setIndCath(3,gdi[2].currC);
	setIndCath(2,gdi[1].currC);
	setIndCath(1,gdi[0].currC);
	
	if(sdvVal == RVE_SDV_ALL)
	{
			fireCurrIndCath(0,LCat[0]|LCat[1]|LCat[2]|
																	LCat[3]|LCat[4]|LCat[5]|
															LCat[6]|LCat[7]|LCat[8]|LCat[9]);
	}
	else if(sdvVal == RVE_SDV_NO)
		offCurrIndCath(0,gdi[0].currC);
	else if(sdvVal == RVE_SDV_CURR)
		setIndCath(0,tmp);
	
	
 }

 void rveSdvigLeft(char sdvVal,char val)
{
	unsigned char tmp = 0;
	tmp = gdi[0].currC;
	setIndCath(0,gdi[1].currC);
	setIndCath(1,gdi[2].currC);
	setIndCath(2,gdi[3].currC);
	setIndCath(3,gdi[4].currC);
	setIndCath(4,gdi[5].currC);
	if(sdvVal == RVE_SDV_ALL)
	{
			fireCurrIndCath(5,LCat[0]|LCat[1]|LCat[2]|
																	LCat[3]|LCat[4]|LCat[5]|
															LCat[6]|LCat[7]|LCat[8]|LCat[9]);
	}
	else if(sdvVal == RVE_SDV_NO)
		offCurrIndCath(5,gdi[5].currC);
	else if(sdvVal == RVE_SDV_CURR)
		setIndCath(5,tmp);
	
		
 }

 void rveFreq(char val1, char val2)
 {
	 TIM2->PSC = val1*val2;
 }
 
 void rveChangeIndBright(char ind, char br)
 {
	 gdi[ind].brAll = br;
 }
 
 void rveDateNTime(char val1, char val2)
 {
	 gdi[5].br[tmdt.sec/10] = gdi[5].brAll;
	 gdi[5].br[tmdt.day/10] = gdi[5].brAll;
	 
	 gdi[4].br[tmdt.sec%10] = gdi[4].brAll;
	 gdi[4].br[tmdt.day%10] = gdi[4].brAll;
	 
	 gdi[3].br[tmdt.min/10] = gdi[3].brAll;
	 gdi[3].br[tmdt.month/10] = gdi[3].brAll;
	 
	 gdi[2].br[tmdt.min%10] = gdi[2].brAll;
	 gdi[2].br[tmdt.month%10] = gdi[2].brAll;
	 
	 gdi[1].br[tmdt.hour/10] = gdi[1].brAll;
	 gdi[1].br[tmdt.year/10] = gdi[1].brAll;
	 
	 gdi[0].br[tmdt.hour%10] = gdi[0].brAll;
	 gdi[0].br[tmdt.year%10] = gdi[0].brAll;
 }
 
 void rveAddCath(char ind,char value)
 {
	  gdi[ind].br[value] = gdi[ind].brAll;
 }
 
 void rveSetBright(char ind, char value)
 {
	 gdi[ind].brAll = value;
	 gdi[ind].br[gdi[ind].currC] = value;
 }
 
 void rveRue(char ind, char value)
 {
	 unsigned char tmp = 0;
	 tmp = gdi[ind].currC+1;
	 if(tmp>9) tmp = 0;
	 
	 gdi[ind].br[gdi[ind].currC] = 0;
	 gdi[ind].currC = tmp;
	 gdi[ind].br[tmp] = gdi[ind].brAll;

	// gdi[ind].currCath = tmp;
 }
 
 void rveOffButComma(char ind,char yark)
 {
	 gdi[ind].br[10] = gdi[ind].brAll;
	 gdi[ind].br[11] = gdi[ind].brAll;
	 
	 gdi[ind].br[gdi[ind].currC] = yark;
	 //gdi[ind].bright[2] = yark;
	 //gdi[ind].bright[3] = yark;
	// gdi[ind].bright[4] = yark;
	// gdi[ind].bright[5] = yark;
	// gdi[ind].bright[6] = yark;
//	 gdi[ind].bright[7] = yark;
	// gdi[ind].bright[8] = yark;
	// gdi[ind].bright[9] = yark;
	// gdi[ind].bright[0] = yark;
 }
 
 void rvePrizrak()
 {
	 
 } 
 
void rveRetBright(unsigned char ind, unsigned char val)
{
	gdi[ind].br[1] = 0;
	gdi[ind].br[2] = 0;
	gdi[ind].br[3] = 0;
	gdi[ind].br[4] = 0;
	gdi[ind].br[5] = 0;
	gdi[ind].br[6] = 0;
	gdi[ind].br[7] = 0;
	gdi[ind].br[8] = 0;
	gdi[ind].br[9] = 0;
	gdi[ind].br[0] = 0;
	gdi[ind].br[10] = 0;
	gdi[ind].br[11] = 0;
	
	gdi[ind].br[gdi[ind].currC] = val;
	
}
void rveRetValue()
{
	setIndCath(0,tmdt.hour/10);
	setIndCath(1,tmdt.hour%10);
	setIndCath(2,tmdt.min/10);
	setIndCath(3,tmdt.min%10);
	setIndCath(4,tmdt.sec/10);
	setIndCath(5,tmdt.sec%10);
}*/
