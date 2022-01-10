#ifndef ADC_h
#define ADC_h

class adc
{
	private:
		int address;
		int config[16];
		int highthresh;
		int lowthresh;
		int pihandle;
		void writeconfig();
		void adcsetup();
	public:
		adc(int addr);
		adc(int addr, int ht, int lt);

		void SetMultiplex(int up, int down);
		void SetMultiplex(int up);
		void SetGain(int gain);
		void SetMode(int mode);
		void SetSPS(int sps);
		void SetCompMode(int mode);
		void SetCompPol(int pol);
		void SetLatch(int latch);
		void SetCompQueue(int que);

		int getreading();

		int gethighthresh();
		int getlowthresh();
		int getconfig();

		~adc();
}
