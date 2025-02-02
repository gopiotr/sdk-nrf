/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "led_widget.h"

#include <dk_buttons_and_leds.h>
#include <zephyr.h>

static LEDWidget::LEDWidgetStateUpdateHandler sStateUpdateCallback;

void LEDWidget::InitGpio()
{
	dk_leds_init();
}

void LEDWidget::SetStateUpdateCallback(LEDWidgetStateUpdateHandler stateUpdateCb)
{
	if (stateUpdateCb)
		sStateUpdateCallback = stateUpdateCb;
}

void LEDWidget::Init(uint32_t gpioNum)
{
	mLastChangeTimeMS = 0;
	mBlinkOnTimeMS = 0;
	mBlinkOffTimeMS = 0;
	mGPIONum = gpioNum;
	mState = false;

	k_timer_init(&mLedTimer, &LEDWidget::LedStateTimerHandler, nullptr);
	k_timer_user_data_set(&mLedTimer, this);

	Set(false);
}

void LEDWidget::Invert()
{
	Set(!mState);
}

void LEDWidget::Set(bool state)
{
	k_timer_stop(&mLedTimer);
	mLastChangeTimeMS = mBlinkOnTimeMS = mBlinkOffTimeMS = 0;
	DoSet(state);
}

void LEDWidget::Blink(uint32_t changeRateMS)
{
	k_timer_stop(&mLedTimer);
	Blink(changeRateMS, changeRateMS);
}

void LEDWidget::Blink(uint32_t onTimeMS, uint32_t offTimeMS)
{
	mBlinkOnTimeMS = onTimeMS;
	mBlinkOffTimeMS = offTimeMS;

	if (mBlinkOnTimeMS != 0 && mBlinkOffTimeMS != 0) {
		DoSet(!mState);
		ScheduleStateChange();
	}
}

void LEDWidget::ScheduleStateChange()
{
	k_timer_start(&mLedTimer, K_MSEC(mState ? mBlinkOnTimeMS : mBlinkOffTimeMS), K_NO_WAIT);
}

void LEDWidget::DoSet(bool state)
{
	mState = state;
	dk_set_led(mGPIONum, state);
}

void LEDWidget::UpdateState()
{
	DoSet(!mState);
	ScheduleStateChange();
}

void LEDWidget::LedStateTimerHandler(k_timer *timer)
{
	if (sStateUpdateCallback)
		sStateUpdateCallback(*reinterpret_cast<LEDWidget *>(timer->user_data));
}
