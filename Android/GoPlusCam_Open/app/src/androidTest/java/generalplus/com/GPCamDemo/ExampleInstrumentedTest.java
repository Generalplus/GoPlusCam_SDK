package generalplus.com.GPCamDemo;
import android.app.Activity;
import android.content.Context;
import android.view.View;
import android.widget.ImageButton;
import android.util.Log;
import androidx.test.espresso.NoMatchingViewException;
import androidx.test.espresso.UiController;
import androidx.test.espresso.ViewAction;
import androidx.test.espresso.action.CoordinatesProvider;
import androidx.test.espresso.action.GeneralClickAction;
import androidx.test.espresso.action.Press;
import androidx.test.espresso.action.Tap;
import androidx.test.espresso.action.ViewActions;
import androidx.test.ext.junit.rules.ActivityScenarioRule;
import androidx.test.platform.app.InstrumentationRegistry;
import androidx.test.ext.junit.runners.AndroidJUnit4;
import androidx.test.filters.LargeTest;

import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;


import static androidx.test.espresso.Espresso.onView;
import static androidx.test.espresso.action.ViewActions.click;
import static androidx.test.espresso.action.ViewActions.closeSoftKeyboard;
import static androidx.test.espresso.action.ViewActions.scrollTo;
import static androidx.test.espresso.action.ViewActions.typeText;
import static androidx.test.espresso.assertion.ViewAssertions.doesNotExist;
import static androidx.test.espresso.assertion.ViewAssertions.matches;
import static androidx.test.espresso.matcher.ViewMatchers.isAssignableFrom;
import static androidx.test.espresso.matcher.ViewMatchers.isDisplayed;
import static androidx.test.espresso.matcher.ViewMatchers.isRoot;
import static androidx.test.espresso.matcher.ViewMatchers.withId;
import static androidx.test.espresso.matcher.ViewMatchers.withText;

import static org.hamcrest.CoreMatchers.not;
import static org.junit.Assert.*;

import androidx.test.espresso.ViewInteraction;
import androidx.test.espresso.matcher.ViewMatchers;

import static androidx.test.espresso.Espresso.onData;
import static androidx.test.espresso.matcher.ViewMatchers.withId;
import static androidx.test.espresso.matcher.ViewMatchers.withText;
import static org.hamcrest.Matchers.allOf;
import static org.hamcrest.Matchers.instanceOf;
import static org.hamcrest.Matchers.is;

import junit.framework.AssertionFailedError;

import org.hamcrest.Matcher;

import java.util.Random;
import java.util.concurrent.TimeUnit;



/**
 * Instrumented test, which will execute on an Android device.
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */
@RunWith(AndroidJUnit4.class)
@LargeTest
public class ExampleInstrumentedTest {

    private static final String TAG = "EspressoTest";
    private static int WAITUI_TIME_OUT = 5000; //ms
    @Test
    public void useAppContext() {
        // Context of the app under test.
        Context appContext = InstrumentationRegistry.getInstrumentation().getTargetContext();
        assertEquals("generalplus.com.GPCamDemo", appContext.getPackageName());
    }

    @Rule
    public ActivityScenarioRule<MainActivity> activityScenarioRule =
            new ActivityScenarioRule<>(MainActivity.class);

    @Test
    public void onTestPlayback() {
        // Type text and then press the button.
        WaitForUI(R.id.imgbtn_connect).perform(click());
        waitTime(5000);
        WaitForUI(R.id.vlc_surface).perform(clickXY(300,300));
        waitTime(1000);
        WaitForUI(R.id.imgbtn_file).perform(click());
        waitTime(5000);

        int testCnt = 1000;
        for (int i = 0; i < testCnt; i++) {
            Log.d(TAG, "Playback test " + i );

            WaitForUI(R.id.gridView1).perform(clickXY(100, 100));
            waitTime(500);

            WaitFoText("播放").perform(click());
            Delay(1000,1500);
            WaitForUI(R.id.vlc_surface);
            pressBack();
            Delay(1000,1000);

        }

    }

    private ViewInteraction WaitFoText(String strTxt){

        long startTime = System.currentTimeMillis();
        long endTime = startTime + WAITUI_TIME_OUT;

        while (System.currentTimeMillis() < endTime) {
            try {
                ViewInteraction viewInteraction = onView(ViewMatchers.withText(strTxt));
                return viewInteraction.check(matches(isDisplayed()));
            } catch (Throwable e) {
                // View is not yet visible, continue waiting
                waitTime(200); // Adjust the delay as needed
            }
        }

        fail("Failed to wait to wait for text: " + strTxt);
        return null;
    }

    private ViewInteraction WaitForUI(int id){

        long startTime = System.currentTimeMillis();
        long endTime = startTime + WAITUI_TIME_OUT;

        while (System.currentTimeMillis() < endTime) {
            try {
                ViewInteraction viewInteraction = onView(ViewMatchers.withId(id));
                return viewInteraction.check(matches(isDisplayed()));
            } catch (Throwable e) {
                // View is not yet visible, continue waiting
                waitTime(200); // Adjust the delay as needed
            }
        }

        fail("Failed to wait to wait for UI: " + id);
        return null;
    }

    private void pressBack() {

        long startTime = System.currentTimeMillis();
        long endTime = startTime + WAITUI_TIME_OUT;

        while (System.currentTimeMillis() < endTime) {
            try {
                ViewInteraction viewInteraction = onView(isRoot());
                viewInteraction.check(matches(isDisplayed()));
                viewInteraction.perform(ViewActions.pressBack());
                return;
            } catch (Throwable e) {
                // View is not yet visible, continue waiting
                waitTime(200); // Adjust the delay as needed
            }
        }

        fail("pressBack failed.");
    }

    private static ViewAction setImageButtonVisibitity(final boolean value) {
        return new ViewAction() {

            @Override
            public Matcher<View> getConstraints() {
                return isAssignableFrom(ImageButton.class);
            }

            @Override
            public void perform(UiController uiController, View view) {
                view.setVisibility(value ? View.VISIBLE : View.GONE);
            }

            @Override
            public String getDescription() {
                return "Show / Hide View";
            }
        };
    }

    private static ViewAction clickXY(final int x, final int y){
        return new GeneralClickAction(
                Tap.SINGLE,
                new CoordinatesProvider() {
                    @Override
                    public float[] calculateCoordinates(View view) {

                        final int[] screenPos = new int[2];
                        view.getLocationOnScreen(screenPos);

                        final float screenX = screenPos[0] + x;
                        final float screenY = screenPos[1] + y;
                        float[] coordinates = {screenX, screenY};

                        return coordinates;
                    }
                },
                Press.FINGER);
    }

    private void Delay(int minMs,int maxMs) {
        Random random = new Random();
        int minDelay = minMs; // 1 second
        int maxDelay = maxMs; // 5 seconds
        int randomDelay = random.nextInt(maxDelay - minDelay + 1) + minDelay;
        waitTime(randomDelay);
    }
    private void waitTime(int waitMillis) {

        try {
            TimeUnit.MILLISECONDS.sleep(waitMillis);
        } catch (InterruptedException ie) {
            // Handle the exception
        }

    }

}