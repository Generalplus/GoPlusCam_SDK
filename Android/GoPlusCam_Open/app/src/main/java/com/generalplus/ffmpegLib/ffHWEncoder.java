package com.generalplus.ffmpegLib;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.media.MediaMuxer;
import android.os.Build;
import android.test.AndroidTestCase;
import android.util.Log;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Timer;
import java.util.TimerTask;

/**
 * Generates a series of video frames, encodes them, decodes them, and tests for significant
 * divergence from the original.
 * <p>
 * We copy the data from the encoder's output buffers to the decoder's input buffers, running
 * them in parallel.  The first buffer output for video/avc contains codec configuration data,
 * which we must carefully forward to the decoder.
 * <p>
 * An alternative approach would be to save the output of the decoder as an mpeg4 video
 * file, and read it back in from disk.  The data we're generating is just an elementary
 * stream, so we'd need to perform additional steps to make that happen.
 */
public class ffHWEncoder extends AndroidTestCase {

    private static final String TAG = "ffHWEncoder";
    private static final boolean VERBOSE = false;           // lots of logging
    // parameters for the encoder
    private static final String MIME_TYPE = "video/avc";    // H.264 Advanced Video Coding

    public final static int HWENCODE_QUALITY_LOW			            = 0x01;
    public final static int HWENCODE_QUALITY_MEDIUM			            = 0x02;
    public final static int HWENCODE_QUALITY_HIGH			            = 0x04;

    private int mFPS = 15;
    private int motionfactor = HWENCODE_QUALITY_HIGH;
    private int mBitRate = 6000000;
    private MediaMuxer mMuxer = null;
    private MediaCodec mEncoder = null;
    private int mColorFormat = -1;
    private boolean mMuxerStarted = false;
    private int mTrackIndex = -1;
    private long mFirstFrameTime = 0;
    private boolean mStop = false;
    private Timer m_Encodetimer=null;

    //--------------------------------------------------------------------------------
    public void SetFPS(int i32FPS)
    {
        mFPS = i32FPS;
        if(mFPS<=0)
            mFPS = 15;
    }
    //--------------------------------------------------------------------------------
    public void SetQuality(int i32Q)
    {
        motionfactor = i32Q;
        if(motionfactor>HWENCODE_QUALITY_HIGH)
            motionfactor = HWENCODE_QUALITY_HIGH;

    }
    //--------------------------------------------------------------------------------
    public void Start(String outputPath ) throws Exception {

        try {

            int res[] = ffmpegWrapper.getInstance().naGetVideoRes();
            int Width = res[0];
            int Height = res[1];

            MediaCodecInfo codecInfo = selectCodec(MIME_TYPE);
            if (codecInfo == null) {
                // Don't fail CTS if they don't have an AVC codec (not here, anyway).
                Log.e(TAG, "Unable to find an appropriate codec for " + MIME_TYPE);
                return;
            }

            //Kush Gauge: pixel count  x motion factor(1, 2 or 4)  x 0.07 = bit rate in bps
            mBitRate = (int)(Width * Height * mFPS * motionfactor * 0.07);

            Log.d(TAG, "found codec: " + codecInfo.getName());
            mColorFormat = selectColorFormat(codecInfo, MIME_TYPE);
            Log.d(TAG, "found colorFormat: " + mColorFormat);
            // We avoid the device-specific limitations on width and height by using values that
            // are multiples of 16, which all tested devices seem to be able to handle.
            MediaFormat format = MediaFormat.createVideoFormat(MIME_TYPE, Width, Height);
            // Set some properties.  Failing to specify some of these can cause the MediaCodec
            // configure() call to throw an unhelpful exception.
            format.setInteger(MediaFormat.KEY_COLOR_FORMAT, mColorFormat);
            format.setInteger(MediaFormat.KEY_BIT_RATE, mBitRate);
            format.setInteger(MediaFormat.KEY_FRAME_RATE, mFPS);
            format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1);

            if(Build.VERSION.SDK_INT  > 23 ) // Only Android 7.0+ support setting high profile
            {
                format.setInteger(MediaFormat.KEY_PROFILE, MediaCodecInfo.CodecProfileLevel.AVCProfileHigh);
            }


            if (VERBOSE) Log.d(TAG, "format: " + format);
            // Create a MediaCodec for the desired codec, then configure it as an encoder with
            // our desired properties.
            mEncoder = MediaCodec.createByCodecName(codecInfo.getName());
            mEncoder.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            mEncoder.start();

            try {
                mMuxer = new MediaMuxer(outputPath, MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4);
            } catch (IOException ioe) {
                throw new RuntimeException("MediaMuxer creation failed", ioe);
            }

            ffmpegWrapper.getInstance().naSetCovertDecodeFrameFormat(ffDecodeFrame.FFDECODE_FORMAT_YUV420P);
            mStop = false;
            mMuxerStarted = false;
            mTrackIndex = -1;

            mFirstFrameTime = System.nanoTime();

            m_Encodetimer = new Timer(true);
            m_Encodetimer.schedule(new EncodeTask(), 0, 40);

        }
        catch (Exception e){
            Release();
        }
    }
    //--------------------------------------------------------------------------------
    public void Stop()
    {
        mStop = true;
    }
    //--------------------------------------------------------------------------------
    private void Release()
    {
        if (VERBOSE) Log.d(TAG, "releasing codecs");
        if (mEncoder != null) {
            mEncoder.stop();
            mEncoder.release();
            mEncoder = null;
        }

        if (mMuxer != null) {
            mMuxer.stop();
            mMuxer.release();
            mMuxer = null;
        }
    }

    //--------------------------------------------------------------------------------
    private class EncodeTask extends TimerTask
    {
        public void run()
        {
            if(mStop)
            {
                EncodeData(null);
                if (m_Encodetimer != null) {
                    m_Encodetimer.cancel();
                    m_Encodetimer = null;
                }
                Release();
            }
            else {
                ffDecodeFrame decodeFrame = ffmpegWrapper.getInstance().naGetDecodeFrame();

                int i32Y = decodeFrame.width * decodeFrame.height;
                int i32U = decodeFrame.width * decodeFrame.height / 4;
                int i32V = decodeFrame.width * decodeFrame.height / 4;

                byte[] YUV420Data = new byte[i32Y + i32U + i32V];


                switch (mColorFormat) {
                    // these are the formats we know how to handle for this test
                    case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar:
                    {
                        System.arraycopy(decodeFrame.data[0], 0, YUV420Data, 0, i32Y);
                        System.arraycopy(decodeFrame.data[1], 0, YUV420Data, i32Y, i32U);
                        System.arraycopy(decodeFrame.data[2], 0, YUV420Data, i32Y+i32U, i32V);
                        break;
                    }
                    case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420PackedPlanar:
                    {
                        break;
                    }
                    case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar:
                    {
                        System.arraycopy(decodeFrame.data[0], 0, YUV420Data, 0, i32Y);

                        // interleave V and U plane
                        for(int i=0;i<i32U;i++)
                        {
                            YUV420Data[i32Y + (2*i)]      = decodeFrame.data[1][i];
                            YUV420Data[i32Y + (2*i)+1 ]   = decodeFrame.data[2][i];
                        }

                        break;
                    }
                    case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420PackedSemiPlanar:
                    {
                        break;
                    }
                    case MediaCodecInfo.CodecCapabilities.COLOR_TI_FormatYUV420PackedSemiPlanar:
                    {
                        break;
                    }
                    default:
                        return;
                }

                EncodeData(YUV420Data);
            }
        }
    };
    //--------------------------------------------------------------------------------
    private void EncodeData(byte[] frameData)
    {
        final int TIMEOUT_USEC = 10000;
        ByteBuffer[] encoderInputBuffers = mEncoder.getInputBuffers();
        ByteBuffer[] encoderOutputBuffers = mEncoder.getOutputBuffers();
        MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();

        // Loop until the output side is done.
        boolean inputDone = false;
        boolean encoderDone = false;
        while (!encoderDone) {
            if (VERBOSE) Log.d(TAG, "loop");
            // If we're not done submitting frames, generate a new one and submit it.  By
            // doing this on every loop we're working to ensure that the encoder always has
            // work to do.
            //
            // We don't really want a timeout here, but sometimes there's a delay opening
            // the encoder device, so a short timeout can keep us from spinning hard.
            if (!inputDone) {
                int inputBufIndex = mEncoder.dequeueInputBuffer(TIMEOUT_USEC);
                if (VERBOSE) Log.d(TAG, "inputBufIndex=" + inputBufIndex);
                if (inputBufIndex >= 0) {
                    long ptsUsec = computePresentationTimeMirco();

                    if(mStop)
                    {
                        mEncoder.queueInputBuffer(inputBufIndex, 0, 0, ptsUsec,
                                MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                        inputDone = true;
                        if (VERBOSE) Log.d(TAG, "sent input EOS (with zero-length frame)");
                    }
                    else
                    {
                        ByteBuffer inputBuf = encoderInputBuffers[inputBufIndex];
                        // the buffer should be sized to hold one full frame
                        assertTrue(inputBuf.capacity() >= frameData.length);
                        inputBuf.clear();
                        inputBuf.put(frameData);
                        mEncoder.queueInputBuffer(inputBufIndex, 0, frameData.length, ptsUsec, 0);
                        if (VERBOSE) Log.d(TAG, "submitted frame to enc");
                        inputDone = true;
                    }

                } else {
                    // either all in use, or we timed out during initial setup
                    if (VERBOSE) Log.d(TAG, "input buffer not available");
                }
            }

            // Check for output from the encoder.  If there's no output yet, we either need to
            // provide more input, or we need to wait for the encoder to work its magic.  We
            // can't actually tell which is the case, so if we can't get an output buffer right
            // away we loop around and see if it wants more input.
            //
            // Once we get EOS from the encoder, we don't need to do this anymore.
            if (!encoderDone) {
                int encoderStatus = mEncoder.dequeueOutputBuffer(info, TIMEOUT_USEC);
                if (encoderStatus == MediaCodec.INFO_TRY_AGAIN_LATER) {
                    // no output available yet
                    if (VERBOSE) Log.d(TAG, "no output from encoder available");
                    encoderDone = true;
                } else if (encoderStatus == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                    // not expected for an encoder
                    encoderOutputBuffers = mEncoder.getOutputBuffers();
                    if (VERBOSE) Log.d(TAG, "encoder output buffers changed");
                } else if (encoderStatus == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                    // not expected for an encoder
                    MediaFormat newFormat = mEncoder.getOutputFormat();
                    if (VERBOSE) Log.d(TAG, "encoder output format changed: " + newFormat);

                    // should happen before receiving buffers, and should only happen once
                    if (mMuxerStarted) {
                        throw new RuntimeException("format changed twice");
                    }

                    // now that we have the Magic Goodies, start the muxer
                    mTrackIndex = mMuxer.addTrack(newFormat);
                    mMuxer.start();
                    mMuxerStarted = true;

                } else if (encoderStatus < 0) {
                    fail("unexpected result from encoder.dequeueOutputBuffer: " + encoderStatus);
                } else { // encoderStatus >= 0
                    ByteBuffer encodedData = encoderOutputBuffers[encoderStatus];
                    if (encodedData == null) {
                        fail("encoderOutputBuffer " + encoderStatus + " was null");
                    }

                    if ((info.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
                        // The codec config data was pulled out and fed to the muxer when we got
                        // the INFO_OUTPUT_FORMAT_CHANGED status.  Ignore it.
                        if (VERBOSE) Log.d(TAG, "ignoring BUFFER_FLAG_CODEC_CONFIG");
                        info.size = 0;
                    }
                    else
                    {
                        encoderDone = (info.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0;
                    }


                    if (info.size != 0) {
                        if (!mMuxerStarted) {
                            throw new RuntimeException("muxer hasn't started");
                        }

                        // adjust the ByteBuffer values to match BufferInfo (not needed?)
                        encodedData.position(info.offset);
                        encodedData.limit(info.offset + info.size);

                        mMuxer.writeSampleData(mTrackIndex, encodedData, info);
                        if (VERBOSE) Log.d(TAG, "sent " + info.size + " bytes to muxer");
                    }

                    mEncoder.releaseOutputBuffer(encoderStatus, false);
                }
            }
        }
    }

    //--------------------------------------------------------------------------------
    /**
     * Returns the first codec capable of encoding the specified MIME type, or null if no
     * match was found.
     */
    private static MediaCodecInfo selectCodec(String mimeType) {
        int numCodecs = MediaCodecList.getCodecCount();
        for (int i = 0; i < numCodecs; i++) {
            MediaCodecInfo codecInfo = MediaCodecList.getCodecInfoAt(i);
            if (!codecInfo.isEncoder()) {
                continue;
            }
            String[] types = codecInfo.getSupportedTypes();
            for (int j = 0; j < types.length; j++) {
                if (types[j].equalsIgnoreCase(mimeType)) {
                    return codecInfo;
                }
            }
        }
        return null;
    }
    //--------------------------------------------------------------------------------
    /**
     * Returns a color format that is supported by the codec and by this test code.  If no
     * match is found, this throws a test failure -- the set of formats known to the test
     * should be expanded for new platforms.
     */
    private static int selectColorFormat(MediaCodecInfo codecInfo, String mimeType) {
        MediaCodecInfo.CodecCapabilities capabilities = codecInfo.getCapabilitiesForType(mimeType);
        for (int i = 0; i < capabilities.colorFormats.length; i++) {
            int colorFormat = capabilities.colorFormats[i];
            if (isRecognizedFormat(colorFormat)) {
                return colorFormat;
            }
        }
        fail("couldn't find a good color format for " + codecInfo.getName() + " / " + mimeType);
        return 0;   // not reached
    }
    //--------------------------------------------------------------------------------
    /**
     * Returns true if this is a color format that this test code understands (i.e. we know how
     * to read and generate frames in this format).
     */
    private static boolean isRecognizedFormat(int colorFormat) {
        switch (colorFormat) {
            // these are the formats we know how to handle for this test
            case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar:
            case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420PackedPlanar:
            case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar:
            case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420PackedSemiPlanar:
            case MediaCodecInfo.CodecCapabilities.COLOR_TI_FormatYUV420PackedSemiPlanar:
                return true;
            default:
                return false;
        }
    }
    //--------------------------------------------------------------------------------
    /**
     * Generates the presentation time for frame N, in mircoseconds.
     */
    private  long computePresentationTimeMirco() {

        long Diff = System.nanoTime() - mFirstFrameTime;

        return (Diff / 1000) ;
    }
}