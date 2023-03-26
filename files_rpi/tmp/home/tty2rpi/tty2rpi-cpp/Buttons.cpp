/*
 * The MIT License (MIT)

 * Copyright (c) 2023 Gary Sweet
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <iostream>
#include <mutex>
#include <cassert>

#include "Buttons.h"

const uint8_t TOP_PIN = 21;
const uint8_t MID_PIN = 16;
const uint8_t BOT_PIN = 20;

const std::array<uint8_t, 3> PINS = { TOP_PIN, MID_PIN, BOT_PIN };

static void Write(const std::string &file, const std::string &data)
{
	FILE *fp = fopen(file.c_str(), "w");
    if (!fp)
        throw "Failed to open GPIO file for writing";
	fwrite(data.c_str(), 1, data.size(), fp);
	fclose(fp);
}

// Note: pins must already have had internal pull-ups enabled for our circuit design.
// We use "raspi-gpio set <pin> ip pu" in the startup script to do this for each one.
static void SetupInput(uint8_t pin)
{
    // Export the pin so we can access its GPIO files
    Write("/sys/class/gpio/export", std::to_string(pin));

    // Buttons drive pins low, so active low
    Write("/sys/class/gpio/gpio" + std::to_string(pin) + "/active_low", "1");

    // Interrupt on rising edge (button press)
    // Without this, poll() won't work
    Write("/sys/class/gpio/gpio" + std::to_string(pin) + "/edge", "rising");
}

// Can't be static as it's a friend
void PollThread(Buttons *buttons)
{
	struct pollfd fds[PINS.size()];

    for (uint32_t i = 0; i < PINS.size(); i++)
    {
        std::string valueFile = "/sys/class/gpio/gpio" + std::to_string(PINS[i]) + "/value";
        fds[i].events = POLLPRI | POLLERR;
        fds[i].fd = open(valueFile.c_str(), O_RDONLY);

        if (fds[i].fd < 0)
            throw "Failed to open gpio value for watching";
    }

    bool first = true;
    while (true)
    {
        int ret = poll(fds, PINS.size(), -1);

        if (ret == -1)
        {
            perror("poll");
            throw "Failed to poll for buttons";
        }

        for (uint32_t i = 0; i < PINS.size(); i++)
        {
            if (fds[i].revents & POLLPRI)
            {
                char v;
                read(fds[i].fd, &v, 1);
                // Note: we always get one set of events up front, so ignore the first
                if (!first)
                    buttons->Pressed(i);
            }
        }
        first = false;
    }
}

Buttons::Buttons()
{
    try
    {
        for (uint8_t pin : PINS)
            SetupInput(pin);

        m_thread = std::thread(PollThread, this);
    }
    catch(const char *e)
    {
        // Non fatal - we just won't have working menu buttons
        std::cerr << "WARNING: " << e << '\n';
    }
}

// NOTE: no destructor or cleanup, this code always just runs forever

void Buttons::Pressed(uint32_t indx)
{
    assert(indx < PINS.size());

    { // Scope the lock
        std::lock_guard lock(m_mutex);
        m_pressedMask |= 1 << indx;
    }

    // Wake any waiters
    if (m_notifyCV)
        m_notifyCV->notify_all();
}

bool Buttons::HasPresses()
{
    std::lock_guard lock(m_mutex);
    return m_pressedMask != 0;
}

uint32_t Buttons::GetPressedMask()
{
    std::lock_guard lock(m_mutex);
    uint32_t mask = m_pressedMask;
    m_pressedMask = 0;
    return mask;
}