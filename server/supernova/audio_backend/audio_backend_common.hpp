//  audio backend helpers
//  Copyright (C) 2010 Tim Blechmann
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; see the file COPYING.  If not, write to
//  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
//  Boston, MA 02111-1307, USA.

#ifndef AUDIO_BACKEND_AUDIO_BACKEND_COMMON_HPP
#define AUDIO_BACKEND_AUDIO_BACKEND_COMMON_HPP

#include <boost/mpl/if.hpp>
#include <boost/static_assert.hpp>

#include "nova-simd/simd_memory.hpp"
#include "nova-tt/dummy_mutex.hpp"
#include "nova-tt/spin_lock.hpp"

#include "utilities/malloc_aligned.hpp"
#include "utilities/sized_array.hpp"


namespace nova
{
namespace detail
{

template <typename sample_type, typename io_sample_type, bool blocking, bool managed_memory = true>
class audio_delivery_helper:
    boost::mpl::if_c<blocking, spin_lock, dummy_mutex>::type
{
    typedef typename boost::mpl::if_c<blocking, spin_lock, dummy_mutex>::type lock_t;
    typedef std::size_t size_t;

public:
    /* to be called from the audio callback */
    /* @{  */
    void deliver_dac_output(const sample_type * source, size_t channel, size_t frames)
    {
        assert(channel < output_samples.size());
        typename lock_t::scoped_lock lock(*this);
        addvec_simd(output_samples[channel].get(), source, frames);
    }

    void deliver_dac_output_64(const sample_type * source, size_t channel)
    {
        assert(channel < output_samples.size());
        typename lock_t::scoped_lock lock(*this);
        addvec_simd<64>(output_samples[channel].get(), source);
    }

    void copy_dac_output(const sample_type * source, size_t channel, size_t frames)
    {
        assert(channel < output_samples.size());
        copyvec_simd(output_samples[channel].get(), source, frames);
    }

    void copy_dac_output_64(const sample_type * source, size_t channel)
    {
        assert(channel < output_samples.size());
        typename lock_t::scoped_lock lock(*this);
        copyvec_simd<64>(output_samples[channel].get(), source);
    }

    void fetch_adc_input(sample_type * destination, size_t channel, size_t frames)
    {
        copyvec_simd(destination, input_samples[channel].get(), frames);
    }

    void fetch_adc_input_64(sample_type * destination, size_t channel)
    {
        copyvec_simd<64>(destination, input_samples[channel].get());
    }
    /* @} */

    /* @{ */
    /** buffers can be directly mapped to the io regions of the host application */
    template <typename Iterator>
    void input_mapping(Iterator const & buffer_begin, Iterator const & buffer_end)
    {
        BOOST_STATIC_ASSERT(!managed_memory);

        size_t input_count = buffer_end - buffer_begin;

        input_samples.resize(input_count);
        std::copy(buffer_begin, buffer_end, input_samples.begin());
    }

    template <typename Iterator>
    void output_mapping(Iterator const & buffer_begin, Iterator const & buffer_end)
    {
        BOOST_STATIC_ASSERT(!managed_memory);

        size_t output_count = buffer_end - buffer_begin;

        output_samples.resize(output_count);
        std::copy(buffer_begin, buffer_end, output_samples.begin());
    }
    /* @} */

protected:
    void clear_inputs(size_t frames_per_tick)
    {
        for (uint16_t channel = 0; channel != input_samples.size(); ++channel)
            zerovec_simd(input_samples[channel].get(), frames_per_tick);
    }

    void clear_outputs(size_t frames_per_tick)
    {
        for (uint16_t channel = 0; channel != output_samples.size(); ++channel)
            zerovec_simd(output_samples[channel].get(), frames_per_tick);
    }

    void prepare_helper_buffers(size_t input_channels, size_t output_channels, size_t frames)
    {
        BOOST_STATIC_ASSERT(managed_memory);

        input_samples.resize(input_channels);
        output_samples.resize(output_channels);
        std::generate(input_samples.begin(), input_samples.end(), boost::bind(calloc_aligned<sample_type>, frames));
        std::generate(output_samples.begin(), output_samples.end(), boost::bind(calloc_aligned<sample_type>, frames));
    }

    sized_array<aligned_storage_ptr<sample_type, managed_memory>,
                aligned_allocator<sample_type> > input_samples, output_samples;
};

class audio_settings_basic
{
protected:
    float samplerate_;
    uint16_t input_channels, output_channels;

public:
    audio_settings_basic(void):
        samplerate_(0.f), input_channels(0), output_channels(0)
    {}

    float get_samplerate(void) const
    {
        return samplerate_;
    }

    uint16_t get_input_count(void) const
    {
        return input_channels;
    }

    uint16_t get_output_count(void) const
    {
        return output_channels;
    }
};

} /* namespace detail */
} /* namespace nova */


#endif /* AUDIO_BACKEND_AUDIO_BACKEND_COMMON_HPP */
