# Bump Wavefile generator

This mini project was conceived for testing a Yamaha Stagepas300 amplifier that cuts out during music performances at seemingly random times.

It was discovered that sometimes a severe bump on a microphone input would make the amplifier fail. I hate to bump expensive microphones hard, so this project creates a wave file that simulates a microphone bump. A rapidly decaying 64.5Hz waveform.

<img src="bump.svg" style="height: 480px; width:600px;"/>

Eventually a smoking gun was found. The negative supply IC TOP249Y pin 2 is connected to the 340V rail two 1M Ohm resistors in series. The resistors are SMD 0402 normally rated at 50V. Their value appears to have reduced to 850k Ohm each, causing the switched mode supply to enter over voltage shutdown mode. The proposed cure is to replace the SMD resistors with two wire ended 1M Ohm 1/4 watt resistors in series.

The Class D amplifier output is monitored for DC offset by Q812 and opto coupler PC802 fed by a low pass filter R860,R861, R877 and C848. This circuit seems to be tuned to detect microphone bump positive excursions! Note that the left and right power amplifiers operate in anti-phase. So this bump generator software generates bumps on left and right channels in all combinations of phase between channels.

If the amplifier passes the synthetic bump test at high mains voltage I will return it to the owner.