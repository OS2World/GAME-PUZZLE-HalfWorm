#pragma pack(2)
typedef struct _SAMPLE
{
   ULONG length;     /* Sample length in words        */
   PSHORT end;       /* sample->data + sample->length (unused) */
   ULONG flags;      /* 0x00000001 - loop (?) (unused) */
   SHORT data[1];    /* Raw waveform data             */
}SAMPLE, *PSAMPLE;
#pragma pack()

BOOL _Optlink isMixerAvailable(void);

BOOL _Optlink startMixer(void);
BOOL _Optlink stopMixer(void);

BOOL _Optlink mixerSetVolume(ULONG ulLevel);

PSAMPLE _Optlink loadSample(PSZ pszFilename);
BOOL _Optlink destroySample(PSAMPLE sample);

BOOL _Optlink playSample(PSAMPLE sample, ULONG ulStartTime);


