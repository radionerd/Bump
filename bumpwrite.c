/*Compiles with gcc -Wall -O2 -o bumpwrite bumpwrite.c */

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/*
The header of a wav file Based on:
https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
*/

// Generates loud decaying bumps for Yamaha Stagepas300 amplifier testing.
// Initially a loud microphone bump of 64.5Hz decaying to half amplitude
// over 1/2 cycle sometimes causes the amplifier to cut out.
// The goal of this wavefile is to stress the amplifier
// without stressing the microphone!
// The fault seems to be related to low value resistance of the over voltage
// detect of the negative supply rail R829, R830 which may have degraded.
// Suspect R829-R830 have 50V rating and are stressed to 340V/2. Oops!
typedef struct wavfile_header_s
{
    char    ChunkID[4];     /*  4   */
    int32_t ChunkSize;      /*  4   */
    char    Format[4];      /*  4   */
    
    char    Subchunk1ID[4]; /*  4   */
    int32_t Subchunk1Size;  /*  4   */
    int16_t AudioFormat;    /*  2   */
    int16_t NumChannels;    /*  2   */
    int32_t SampleRate;     /*  4   */
    int32_t ByteRate;       /*  4   */
    int16_t BlockAlign;     /*  2   */
    int16_t BitsPerSample;  /*  2   */
    
    char    Subchunk2ID[4];
    int32_t Subchunk2Size;
} wavfile_header_t;

/*Standard values for CD-quality audio*/
#define SUBCHUNK1SIZE   (16)
#define AUDIO_FORMAT    (1) /*For PCM*/
#define NUM_CHANNELS    (2)
#define SAMPLE_RATE     (44100)

#define BITS_PER_SAMPLE (16)

#define BYTE_RATE       (SAMPLE_RATE * NUM_CHANNELS * BITS_PER_SAMPLE / 8)
#define BLOCK_ALIGN     (NUM_CHANNELS * BITS_PER_SAMPLE / 8)

/*Return 0 on success and -1 on failure*/
int write_PCM16_stereo_header(FILE *file_p,
                                int32_t SampleRate,
                                int32_t FrameCount)
{
    int ret;
    
    wavfile_header_t wav_header;
    int32_t subchunk2_size;
    int32_t chunk_size;
    
    size_t write_count;
    
    subchunk2_size  = FrameCount * NUM_CHANNELS * BITS_PER_SAMPLE / 8;
    chunk_size      = 4 + (8 + SUBCHUNK1SIZE) + (8 + subchunk2_size);
    
    wav_header.ChunkID[0] = 'R';
    wav_header.ChunkID[1] = 'I';
    wav_header.ChunkID[2] = 'F';
    wav_header.ChunkID[3] = 'F';
    
    wav_header.ChunkSize = chunk_size;
    
    wav_header.Format[0] = 'W';
    wav_header.Format[1] = 'A';
    wav_header.Format[2] = 'V';
    wav_header.Format[3] = 'E';
    
    wav_header.Subchunk1ID[0] = 'f';
    wav_header.Subchunk1ID[1] = 'm';
    wav_header.Subchunk1ID[2] = 't';
    wav_header.Subchunk1ID[3] = ' ';
    
    wav_header.Subchunk1Size = SUBCHUNK1SIZE;
    wav_header.AudioFormat = AUDIO_FORMAT;
    wav_header.NumChannels = NUM_CHANNELS;
    wav_header.SampleRate = SampleRate;
    wav_header.ByteRate = BYTE_RATE;
    wav_header.BlockAlign = BLOCK_ALIGN;
    wav_header.BitsPerSample = BITS_PER_SAMPLE;
    
    wav_header.Subchunk2ID[0] = 'd';
    wav_header.Subchunk2ID[1] = 'a';
    wav_header.Subchunk2ID[2] = 't';
    wav_header.Subchunk2ID[3] = 'a';
    wav_header.Subchunk2Size = subchunk2_size;
    
    write_count = fwrite(   &wav_header, 
                            sizeof(wavfile_header_t), 1,
                            file_p);
                    
    ret = (1 != write_count)? -1 : 0;
    
    return ret;
}

/*Data structure to hold a single frame with two channels*/
typedef struct PCM16_stereo_s
{
    int16_t left;
    int16_t right;
} PCM16_stereo_t;

PCM16_stereo_t *allocate_PCM16_stereo_buffer(int32_t FrameCount)
{
    return (PCM16_stereo_t *)malloc(sizeof(PCM16_stereo_t) * FrameCount);
}


/*Return the number of audio frames sucessfully written*/
size_t  write_PCM16wav_data(FILE*           file_p,
                            int32_t         FrameCount,
                            PCM16_stereo_t  *buffer_p)
{
    size_t ret;
    
    ret = fwrite(   buffer_p, 
                    sizeof(PCM16_stereo_t), FrameCount,
                    file_p);
                    
    return ret;
}

/*Generate two saw-tooth signals at two frequencies and amplitudes*/
int generate_dual_sawtooth( double frequency1,
                            double amplitude1,
                            double frequency2,
                            double amplitude2,
                            int32_t SampleRate,
                            int32_t FrameCount,
                            PCM16_stereo_t  *buffer_p)
{
    int ret = 0;
    double SampleRate_d = (double)SampleRate;
    double SamplePeriod = 1.0 / SampleRate_d;
    
    double Period1, Period2;
    double phase1, phase2;
    double Slope1, Slope2;
    
    int32_t k;
    
    /*Check for the violation of the Nyquist limit*/
    if( (frequency1*2 >= SampleRate_d) || (frequency2*2 >= SampleRate_d) )
    {
        ret = -1;
        goto error0;
    }
    
    /*Compute the period*/
    Period1 = 1.0 / frequency1;
    Period2 = 1.0 / frequency2;
    
    /*Compute the slope*/
    Slope1  = amplitude1 / Period1;
    Slope2  = amplitude2 / Period2;
    
    for(k = 0, phase1 = 0.0, phase2 = 0.0; 
        k < FrameCount; 
        k++)
    {
        phase1 += SamplePeriod;
        phase1 = (phase1 > Period1)? (phase1 - Period1) : phase1;
        
        phase2 += SamplePeriod;
        phase2 = (phase2 > Period2)? (phase2 - Period2) : phase2;
        
        buffer_p[k].left    = (int16_t)(phase1 * Slope1);
        buffer_p[k].right   = (int16_t)(phase2 * Slope2);
    }
    
error0:
    return ret;
}

/*Generate stereo bump with exponential decay signal */
int generate_bump( double frequency,
                            double amplitude_left,
                            double amplitude_right,
                            double decay_rate,
                            int32_t SampleRate,
                            int32_t FrameCount,
                            PCM16_stereo_t  *buffer_p)
{
    int ret = 0;
        
    /*Check for the violation of the Nyquist limit*/
    if( (frequency * 2 >= SampleRate) )
    {
        ret = -1;
        goto error0;
    }
    
    double phase = 0.0;
    double phase_increment = 2 * M_PI / ( SampleRate / frequency );
       
    for( int32_t i = 0; i < FrameCount; i++ )
    {
        double sample = sin ( phase ) * exp ( -i / decay_rate )  ; 
        phase += phase_increment;
        buffer_p[i].left    = (int16_t)(sample * amplitude_left);
        buffer_p[i].right   = (int16_t)(sample * amplitude_right);
    }
    
error0:
    return ret;
}



int main(void)
{
    int ret;
    FILE* file_p;

    double frequency  = 64.5; /*B4*/
    double amplitude_left  = 1.0 * (double)SHRT_MAX;
    double amplitude_right  = 1.0 * (double)SHRT_MAX;
    double decay_rate = 680;
  
    double duration = 1; /*seconds*/
    int32_t FrameCount = duration * SAMPLE_RATE;
    
    PCM16_stereo_t  *buffer_p = NULL;
    
    size_t written;
    
    /*Open the wav file*/
    file_p = fopen("./bump.wav", "wb");
    if(NULL == file_p)
    {
        perror("fopen failed in main");
        ret = -1;
        goto error0;
    }
    
    /*Allocate the data buffer*/
    buffer_p = allocate_PCM16_stereo_buffer(FrameCount);
    if(NULL == buffer_p)
    {
        perror("fopen failed in main");
        ret = -1;
        goto error1;        
    }

    /*Fill the buffer*/
    uint32_t FrameOffset = 0;
    int bumps = 10;
    ret = generate_bump ( frequency, 0.0           , 0.0            , decay_rate, 
                          SAMPLE_RATE, FrameCount/bumps, FrameOffset+ buffer_p);
    FrameOffset += FrameCount/bumps;
    ret = generate_bump ( frequency, amplitude_left, amplitude_right, decay_rate, 
                          SAMPLE_RATE, FrameCount/bumps, FrameOffset+buffer_p);
    FrameOffset += FrameCount/bumps;
    ret = generate_bump ( frequency,-amplitude_left, amplitude_right, decay_rate, 
                          SAMPLE_RATE, FrameCount/bumps, FrameOffset+ buffer_p);
    FrameOffset += FrameCount/bumps;
    ret = generate_bump ( frequency, amplitude_left, -amplitude_right, decay_rate, 
                          SAMPLE_RATE, FrameCount/bumps, FrameOffset+buffer_p);
    FrameOffset += FrameCount/bumps;
    ret = generate_bump ( frequency,-amplitude_left, -amplitude_right, decay_rate, 
                          SAMPLE_RATE, FrameCount/bumps, FrameOffset+ buffer_p);
    FrameOffset += FrameCount/bumps;
    ret = generate_bump ( frequency, amplitude_left, 0.0            , decay_rate, 
                          SAMPLE_RATE, FrameCount/bumps, FrameOffset+ buffer_p);
    FrameOffset += FrameCount/bumps;
    ret = generate_bump ( frequency,-amplitude_left, 0.0            , decay_rate, 
                          SAMPLE_RATE, FrameCount/bumps, FrameOffset+ buffer_p);
    FrameOffset += FrameCount/bumps;
    ret = generate_bump ( frequency, 0.0           , amplitude_right, decay_rate, 
                          SAMPLE_RATE, FrameCount/bumps, FrameOffset+ buffer_p);
    FrameOffset += FrameCount/bumps;
    ret = generate_bump ( frequency, 0.0           ,-amplitude_right, decay_rate, 
                          SAMPLE_RATE, FrameCount/bumps, FrameOffset+ buffer_p);
    FrameOffset += FrameCount/bumps;
    ret = generate_bump ( frequency, 0.0           , 0.0            , decay_rate, 
                          SAMPLE_RATE, FrameCount/bumps, FrameOffset+ buffer_p);
    /*ret = generate_dual_sawtooth(   frequency1,
                                    amplitude1,
                                    frequency2,
                                    amplitude2,
                                    SAMPLE_RATE,
                                    FrameCount,
                                    buffer_p);*/
    if(ret < 0)
    {
        fprintf(stderr, "generate samples failed in main\n");
        ret = -1;
        goto error2;
    }
    
    /*Write the wav file header*/
    ret = write_PCM16_stereo_header(file_p,
                                    SAMPLE_RATE,
                                    FrameCount);
    if(ret < 0)
    {
        perror("write_PCM16_stereo_header failed in main");
        ret = -1;
        goto error2;
    }
    
    /*Write the data out to file*/
    written = write_PCM16wav_data(  file_p,
                                    FrameCount,
                                    buffer_p);
    if(written < FrameCount)
    {
        perror("write_PCM16wav_data failed in main");
        ret = -1;
        goto error2;
    }

    /*Free and close everything*/    
error2:
    free(buffer_p);
error1:
    fclose(file_p);
error0:
    return ret;    
}

