#pragma strings(readonly)

#define INCL_DOSSEMAPHORES
#define INCL_DOSPROCESS
#define INCL_DOSERRORS
#define INCL_OS2MM

#include <os2.h>
#include <os2me.h>

#include <memory.h>
#include <malloc.h>
#include <stdlib.h>
#include <process.h>

#include "SoundEngine.h"

#include "debug.h"


/* Maximum channels */
#define MAX_CHANNELS   8

/* Buffer properties */
#define MIX_BUFFERS                      4
#define MIX_BUFFER_SIZE                  256


typedef struct _CHANNEL
{
   PSAMPLE sample;                       /* NULL for inactive channels */
   PSHORT pRead;                         /* Read position pointer */
   ULONG remaining;                      /* remaining words */
   int iWriteBuffer;                     /* Which buffer will be written to */
   ULONG ulStartTime;                    /* When sample started playing */
}CHANNEL, *PCHANNEL;


/*
 * MMPM/2 DART buffers/variables
 */
USHORT usDeviceID = 0;
MCI_MIXSETUP_PARMS MixSetupParms = { 0 };
MCI_BUFFER_PARMS BufferParms = { 0 };
PMCI_MIX_BUFFER MixBuffers = NULL;
int nBufferWords = 0;

/*
 * Channel and DART buffer variables
 */
int iPlayingBuffer = 0;
int iNextBuffer = 0;
CHANNEL channels[MAX_CHANNELS] = { NULL };

/*
 * Mixer thread communication variables
 */
int tidMixer = -1;
HEV hevMix = NULLHANDLE;
HEV hevTermMixer = NULLHANDLE;
HEV hevMixerTerminated = NULLHANDLE;



/*
 * Function prototypes - local functions
 */
static void _Optlink mixerThread(void *param);

static BOOL _Optlink dartInitMixer(void);
static BOOL _Optlink dartTermMixer(void);

static LONG APIENTRY dartEventHandler(ULONG ulStatus, PMCI_MIX_BUFFER pBuffer, ULONG flags);


BOOL _Optlink isMixerAvailable(void)
{
   BOOL fAvailable = FALSE;
   MCI_AMP_OPEN_PARMS AmpOpenParms = { 0 };
   ULONG rc = MCIERR_SUCCESS;

   memset(&AmpOpenParms, 0, sizeof(MCI_AMP_OPEN_PARMS));
   AmpOpenParms.pszDeviceType = (PSZ)MCI_DEVTYPE_AUDIO_AMPMIX;
   if((rc = mciSendCommand(0, MCI_OPEN, MCI_WAIT | MCI_OPEN_SHAREABLE | MCI_OPEN_TYPE_ID, &AmpOpenParms, 0)) == MCIERR_SUCCESS)
   {
      MCI_MIXSETUP_PARMS msp = { 0 };
      MCI_GENERIC_PARMS GenericParms = { 0 };

      memset(&msp, 0, sizeof(msp));
      msp.ulBitsPerSample = BPS_16;
      msp.ulFormatTag = DATATYPE_WAVEFORM;
      msp.ulSamplesPerSec = HZ_22050;
      msp.ulChannels = CH_1;
      msp.ulFormatMode = MCI_PLAY;
      msp.ulDeviceType = MCI_DEVTYPE_WAVEFORM_AUDIO;
      msp.pmixEvent = dartEventHandler;
      if((rc = mciSendCommand(AmpOpenParms.usDeviceID, MCI_MIXSETUP, MCI_WAIT | MCI_MIXSETUP_INIT, &msp, 0)) == MCIERR_SUCCESS)
      {
         fAvailable = TRUE;
      }
      mciSendCommand(AmpOpenParms.usDeviceID, MCI_CLOSE, MCI_WAIT, &GenericParms, 0);
   }
   return fAvailable;
}




BOOL _Optlink startMixer(void)
{
   APIRET rc = NO_ERROR;
   BOOL fSuccess = FALSE;

   rc = DosCreateEventSem(NULL, &hevMix, 0UL, FALSE);
   if(rc == NO_ERROR)
   {
      rc = DosCreateEventSem(NULL, &hevTermMixer, 0UL, FALSE);
   }
   if(rc == NO_ERROR)
   {
      rc = DosCreateEventSem(NULL, &hevMixerTerminated, 0UL, FALSE);
   }

   if(rc == NO_ERROR)
   {
      /* Intialize DART (device & buffers) */
      if(dartInitMixer())
      {
         tidMixer = _beginthread(mixerThread, NULL, 8192, NULL);
         dprintf(("tidMixer = %d\n", tidMixer));
         if(tidMixer != -1)
            fSuccess = TRUE;
      }
      else
      {
         dputs(("e dartInitMixer failed!"));
      }
   }
   if(tidMixer == -1)
   {
      stopMixer();
   }
   return fSuccess;
}

BOOL _Optlink stopMixer(void)
{
   BOOL fSuccess = FALSE;
   APIRET rc = NO_ERROR;
   MCI_GENERIC_PARMS GenericParms = { 0 };

   mciSendCommand(usDeviceID, MCI_STOP, MCI_WAIT, &GenericParms, 0);

   /*
    * Nota bene: There may be leftover in the mixer buffers, need to clear them?
    */

   /*
    * Terminate the mixer thread
    */
   if((rc = DosPostEventSem(hevTermMixer)) == NO_ERROR)
   {
      DosPostEventSem(hevMix);

      if((rc = DosWaitEventSem(hevMixerTerminated, 2000)) == NO_ERROR)
      {
         tidMixer = -1;
      }
      else if(rc == ERROR_TIMEOUT)
      {
         if((rc = DosKillThread(tidMixer)) == NO_ERROR)
         {
            tidMixer = -1;
         }
      }
   }

   if(dartTermMixer())
   {
      fSuccess = TRUE;
   }


   /*
    * Destroy semaphores
    */
   if((rc = DosCloseEventSem(hevMixerTerminated)) == NO_ERROR)
   {
      hevMixerTerminated = NULLHANDLE;
   }
   if((rc = DosCloseEventSem(hevTermMixer)) == NO_ERROR)
   {
      hevTermMixer = NULLHANDLE;
   }
   if((rc = DosCloseEventSem(hevMix)) == NO_ERROR)
   {
      hevMix = NULLHANDLE;
   }
   return fSuccess;
}

BOOL _Optlink mixerSetVolume(ULONG ulLevel)
{
   MCI_SET_PARMS      msp;

   msp.ulLevel = ulLevel;
   msp.ulAudio = MCI_SET_AUDIO_ALL;
   mciSendCommand(usDeviceID, MCI_SET, MCI_WAIT | MCI_SET_AUDIO | MCI_SET_VOLUME, &msp, 0);
}



PSAMPLE _Optlink loadSample(PSZ pszFilename)
{
   MMIOINFO mmioInfo = { 0 };
   HMMIO hmmio = NULLHANDLE;
   BOOL fSuccess = FALSE;
   PSAMPLE sample = NULL;

   memset(&mmioInfo, '\0', sizeof(MMIOINFO));
   mmioInfo.fccIOProc = mmioFOURCC('W', 'A', 'V', 'E');
   hmmio = mmioOpen(pszFilename, &mmioInfo, MMIO_READ | MMIO_DENYNONE );
   if(hmmio != NULLHANDLE)
   {
      ULONG rc = MMIO_SUCCESS;
      LONG lBytesRead = 0L;

      MMAUDIOHEADER mmAudioHeader = { 0 };
      rc = mmioGetHeader(hmmio, &mmAudioHeader, sizeof(MMAUDIOHEADER), &lBytesRead, 0, 0);
      if(rc == MMIO_SUCCESS)
      {
         if(mmAudioHeader.mmXWAVHeader.WAVEHeader.usChannels != CH_1 ||
            mmAudioHeader.mmXWAVHeader.WAVEHeader.ulSamplesPerSec != HZ_22050 ||
            mmAudioHeader.mmXWAVHeader.WAVEHeader.usBitsPerSample != BPS_16)
         {
            mmioClose(hmmio, 0);
            return NULL;
         }
         sample = malloc(sizeof(SAMPLE)+mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes-sizeof(SHORT));
         if(sample)
         {
            lBytesRead = mmioRead(hmmio, (char*)sample->data, mmAudioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes);
            sample->length = lBytesRead/2;
            sample->end = sample->data + lBytesRead;
         }
         mmioClose(hmmio, 0);
      }
   }
   return sample;
}

BOOL _Optlink destroySample(PSAMPLE sample)
{
   free(sample);
   _heapmin();
   return TRUE;
}


/*
 * Add a sample to the channel list
 */
BOOL _Optlink playSample(PSAMPLE sample, ULONG ulStartTime)
{
   int iCh = 0;
   PCHANNEL ch = NULL;
   int iOldest = MAX_CHANNELS+1;
   ULONG ulOldestTime = 0;

   /* Quick exit? */
   if(sample == NULL)
      return FALSE;

   for(; iCh < MAX_CHANNELS; iCh++)
   {
      if(channels[iCh].sample == NULL)
      {
         ch = &channels[iCh];
         break;
      }

      if(ulOldestTime > channels[iCh].ulStartTime || iOldest == MAX_CHANNELS+1)
      {
         /* Determine which channel/sample is oldest */
         iOldest = iCh;
         ulOldestTime = channels[iCh].ulStartTime;
      }
   }

   if(!ch)
   {
      ch = &channels[iOldest];
   }

   ch->pRead = sample->data;
   ch->remaining = sample->length;
   ch->ulStartTime = ulStartTime;
   ch->sample = sample;  /* Last, but not least */

   return TRUE;
}












/*
 * The mixer thread
 * Stupid Design v1.0
 *
 * Okey, this is a quick hack to attempt to get sound in HalfWorm.
 * The sound engine keeps track of a list of "channels" which contain pointers
 * to sample data buffers. The channel-sample-data is mixed together into the
 * DART buffers in this routine. This method of mixing sound in realtime is
 * bad, I need to write a real sound engine, but for now I just want sound.
 */
static void _Optlink mixerThread(void *param)
{
   APIRET rc = NO_ERROR;

   /* Wait for a mix event; could be coming from playSample or dart event
      handler */
   while((rc = DosWaitEventSem(hevMix, (ULONG)SEM_INDEFINITE_WAIT)) == NO_ERROR)
   {
      ULONG cPosts = 0;
      int iCh = 0;

      if((rc = DosWaitEventSem(hevTermMixer, SEM_IMMEDIATE_RETURN)) == NO_ERROR)
      {
         /* break out of loop */
         break;
      }

      /* Reset semaphore before processing to make sure no mixing is missed */
      DosResetEventSem(hevMix, &cPosts);

      /* Loop through all channels */
      for(; iCh < MAX_CHANNELS; iCh++)
      {
         PCHANNEL ch = &channels[iCh];
         int nWrite;
         PSHORT pWrite;
         PSHORT pWriteEnd;

         /* Skip channels which aren't active */
         if(ch->sample == NULL)
            continue;

         if(ch->iWriteBuffer == iPlayingBuffer)
         {
            /*
             * NOTE: In some cases, a buffer will not have time to get mixed
             *       into it's designated buffer before DART begins using that
             *       buffer. In those cases, make sure sample is written into
             *       next buffer instread. This may result in an annoying
             *       delay.
             *       TODO: Find a better way of dealing with this issue!
             */
            if(ch->remaining == ch->sample->length)
            {
               /* Sample has not been mixed at all, and it was supposed to be
                * written to the buffer currently being sent to the mixer.
                * Make sure it's written in next buffer instead. */
               ch->iWriteBuffer = iNextBuffer;
               iCh--;
            }

            /* Process next or reprocess current */
            continue;
         }

         /* If this point has been reached, then channel has data which should
          * be mixed into the DART buffers. Each channel keeps track of which
          * buffer it wrote to last time. */

         /* TODO: If channel missed a buffer, increase priority of thread */

         /* Start writing at the beginning of the buffer */
         pWrite = (PSHORT)MixBuffers[ch->iWriteBuffer].pBuffer;

         /* Calculate how many words are to be written */
         nWrite = min(nBufferWords, ch->remaining);
         pWriteEnd = pWrite + nWrite;

         /* Write sample buffer to mixer buffer */
         while(pWrite != pWriteEnd)
         {
            int outSample = *pWrite;

            outSample += *ch->pRead++;

            *pWrite = (SHORT)min(0x7fff, max(outSample, -0x8000));

            pWrite++;
         }

         ch->remaining -= nWrite;
         if(ch->remaining == 0)
         {
            /* All sample data has been written, make channel available for
             * a new sample */
            ch->sample = NULL;
         }
         else
         {
            /* Select next buffer to be written */
            ch->iWriteBuffer = (ch->iWriteBuffer+1) % BufferParms.ulNumBuffers;
         }
      }
   }
   DosPostEventSem(hevMixerTerminated);

   _endthread();
}



static BOOL _Optlink dartInitMixer(void)
{
   MCI_AMP_OPEN_PARMS AmpOpenParms = { 0 };
   ULONG rc = MCIERR_SUCCESS;
   BOOL fSuccess = FALSE;

   /*
    * Open amp mixer device
    * ToDo: Sharing!
    */
   memset(&AmpOpenParms, 0, sizeof(MCI_AMP_OPEN_PARMS));
   AmpOpenParms.pszDeviceType = (PSZ)MCI_DEVTYPE_AUDIO_AMPMIX;
   if((rc = mciSendCommand(0, MCI_OPEN, MCI_WAIT | MCI_OPEN_SHAREABLE | MCI_OPEN_TYPE_ID, &AmpOpenParms, 0)) == MCIERR_SUCCESS)
   {
      usDeviceID = AmpOpenParms.usDeviceID;

      dprintf(("amplifier/mixer device id=%04x\n", usDeviceID));

      /*
       * Set up mixer
       */
      memset(&MixSetupParms, 0, sizeof(MixSetupParms));
      MixSetupParms.ulBitsPerSample = BPS_16;
      MixSetupParms.ulFormatTag = DATATYPE_WAVEFORM;
      MixSetupParms.ulSamplesPerSec = HZ_22050;
      MixSetupParms.ulChannels = CH_1;
      MixSetupParms.ulFormatMode = MCI_PLAY;
      MixSetupParms.ulDeviceType = MCI_DEVTYPE_WAVEFORM_AUDIO;
      MixSetupParms.pmixEvent = dartEventHandler;
      if((rc = mciSendCommand(usDeviceID, MCI_MIXSETUP, MCI_WAIT | MCI_MIXSETUP_INIT, &MixSetupParms, 0)) == MCIERR_SUCCESS)
      {
         /*
          * ulNumBuffers and ulBufferSize now contains recommended values, but I
          * want better "response time" (ie. smaller and few buffers).
          */
         MixSetupParms.ulNumBuffers = MIX_BUFFERS;
         MixSetupParms.ulBufferSize = MIX_BUFFER_SIZE;

         /* Allocate buffers */
         MixBuffers = calloc(MixSetupParms.ulNumBuffers, sizeof(MCI_MIX_BUFFER));

         BufferParms.ulNumBuffers = MixSetupParms.ulNumBuffers;
         BufferParms.ulBufferSize = MixSetupParms.ulBufferSize;
         BufferParms.pBufList = MixBuffers;
         if((rc = mciSendCommand(usDeviceID, MCI_BUFFER, MCI_WAIT | MCI_ALLOCATE_MEMORY, &BufferParms, 0)) == MCIERR_SUCCESS)
         {
            int i = 0;

            nBufferWords = BufferParms.ulBufferSize/2;

            for(i = 0; i < BufferParms.ulNumBuffers; i++)
            {
               MixBuffers[i].ulBufferLength = BufferParms.ulBufferSize;
               memset(MixBuffers[i].pBuffer, 0x00, BufferParms.ulBufferSize);
            }

            fSuccess = TRUE;

            /* Kick off play routine */
            MixSetupParms.pmixWrite(MixSetupParms.ulMixHandle, MixBuffers, 2);
         }
      }
   }

   return fSuccess;
}


static BOOL _Optlink dartTermMixer(void)
{
   BOOL fSuccess = FALSE;
   ULONG rc = MCIERR_SUCCESS;
   MCI_GENERIC_PARMS GenericParms = { 0 };

   if((rc = mciSendCommand(usDeviceID, MCI_BUFFER, MCI_WAIT | MCI_DEALLOCATE_MEMORY, &BufferParms, 0)) == MCIERR_SUCCESS)
   {
   }

   if((rc = mciSendCommand(usDeviceID, MCI_CLOSE, MCI_WAIT, &GenericParms, 0)) == MCIERR_SUCCESS)
   {
      fSuccess = TRUE;
   }
   return fSuccess;
}


static LONG APIENTRY dartEventHandler(ULONG ulStatus, PMCI_MIX_BUFFER pBuffer, ULONG flags)
{
   PSHORT p;
   ULONG i = 0;
   switch(flags)
   {
      case MIX_STREAM_ERROR | MIX_READ_COMPLETE:
      case MIX_STREAM_ERROR | MIX_WRITE_COMPLETE:
         iPlayingBuffer = iNextBuffer;
         iNextBuffer = (iPlayingBuffer+1) % BufferParms.ulNumBuffers;
         break;

      case MIX_WRITE_COMPLETE:
         /* Clear buffer which was written successfully */
         memset(MixBuffers[iPlayingBuffer].pBuffer, 0x00, BufferParms.ulBufferSize);

         /* Write next buffer to mixer (hope it has been prepared by the mixer thread....) */
         MixSetupParms.pmixWrite(MixSetupParms.ulMixHandle, &MixBuffers[iNextBuffer], 1);

         /* Advance index values so right buffer get's written */
         iPlayingBuffer = iNextBuffer;
         iNextBuffer = (iPlayingBuffer+1) % BufferParms.ulNumBuffers;

         /* Tell mixer thread to take a turn (since there's a new buffer has become available) */
         DosPostEventSem(hevMix);
         break;
   }
   return TRUE;
}
