/*
 *  This file is part of RawTherapee.
 *
 *  Copyright (C) 2020 Flössie <floessie.mail@gmail.com>
 *
 *  RawTherapee is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  RawTherapee is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with RawTherapee.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <cstddef>
#include <tuple>

#include <glibmm/main.h>
#include <glibmm/signalproxy.h>

template<typename... Ts>
class DelayedConnection final
{
public:
    DelayedConnection(unsigned int _min_delay_ms, unsigned int _max_delay_ms = 0) :
        min_delay_ms(_min_delay_ms),
        max_delay_ms(_max_delay_ms)
    {
    }

    void connect(Glib::SignalProxy<void, Ts...> signal, const sigc::slot<void, Ts...>& slot, const sigc::slot<void, Ts...>& immediate_slot = {})
    {
        this->slot = slot;
        this->immediate_slot = immediate_slot;
        this->signal = signal.connect(sigc::mem_fun(*this, &DelayedConnection::onSignal));
    }

    void block(bool value = true)
    {
        signal.block(value);
        min_timeout.block(value);
        max_timeout.block(value);
    }

    void unblock()
    {
        signal.unblock();
        min_timeout.unblock();
        max_timeout.unblock();
    }

    void cancel()
    {
        min_timeout.disconnect();
        max_timeout.disconnect();
    }

    void setDelay(unsigned int min_delay_ms, unsigned int max_delay_ms = 0)
    {
        this->min_delay_ms = min_delay_ms;
        this->max_delay_ms = max_delay_ms;

        min_timeout.disconnect();
        max_timeout.disconnect();
    }

private:
    void onSignal(Ts... ts)
    {
        if (immediate_slot) {
            immediate_slot(ts...);
        }

        if (!min_delay_ms) {
            slot(ts...);
            return;
        }

        params = std::make_tuple(ts...);

        min_timeout.disconnect();
        min_timeout = Glib::signal_timeout().connect(sigc::mem_fun(*this, &DelayedConnection::onMinTimeout), min_delay_ms);

        if (max_delay_ms && !max_timeout.connected()) {
            max_timeout = Glib::signal_timeout().connect(sigc::mem_fun(*this, &DelayedConnection::onMaxTimeout), max_delay_ms);
        }
    }

    bool onMinTimeout()
    {
        max_timeout.disconnect();
        apply(params, slot);
        return false;
    }

    bool onMaxTimeout()
    {
        min_timeout.disconnect();
        apply(params, slot);
        return false;
    }

    // C++14

    // See https://gist.github.com/ntessore/dc17769676fb3c6daa1f
    template<std::size_t... Is>
    struct index_sequence
    {
    };

    template<std::size_t N, std::size_t... Is>
    struct make_index_sequence :
        make_index_sequence<N-1, N-1, Is...>
    {
    };

    template<std::size_t... Is>
    struct make_index_sequence<0, Is...> :
        index_sequence<Is...>
    {
    };

    // C++17

    // See https://aherrmann.github.io/programming/2016/02/28/unpacking-tuples-in-cpp14/
    template<typename T, class F, size_t... Is>
    constexpr void apply_impl(T t, F f, index_sequence<Is...>)
    {
        f(std::get<Is>(t)...);
    }

    template <class T, class F>
    constexpr void apply(T t, F f)
    {
        apply_impl(t, f, make_index_sequence<std::tuple_size<T>{}>{});
    }

    unsigned int min_delay_ms;
    unsigned int max_delay_ms;

    sigc::connection signal;
    sigc::connection min_timeout;
    sigc::connection max_timeout;

    sigc::slot<void, Ts...> slot;
    sigc::slot<void, Ts...> immediate_slot;

    std::tuple<Ts...> params;
};
