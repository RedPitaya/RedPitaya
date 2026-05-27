#ifndef SETTINGS_LIB_CHANNELS_H
#define SETTINGS_LIB_CHANNELS_H

#include <bit>
#include <cstdint>
#include <iostream>
#include <utility>

#include <type_traits>

#define MAX_DAC_CHANNELS 2
#define MAX_ADC_CHANNELS 4

enum class DACChannels { DAC_CH1 = 0, DAC_CH2 = 1 };
enum class ADCChannels { ADC_CH1 = 0, ADC_CH2 = 1, ADC_CH3 = 2, ADC_CH4 = 3 };

template <typename Enum, size_t MaxChannels>
struct Channels {
    static_assert(std::is_enum_v<Enum>, "Enum must be an enumeration type");

	using EnumType = Enum;

	using StorageType =
        std::conditional_t<(MaxChannels <= 8), uint8_t, std::conditional_t<(MaxChannels <= 16), uint16_t, std::conditional_t<(MaxChannels <= 32), uint32_t, uint64_t>>>;

    StorageType activeMask = 0;

    Channels() = default;

    Channels(const Channels& other) : activeMask(other.activeMask) {}

    Channels(Channels&& other) noexcept : activeMask(std::exchange(other.activeMask, 0)) {}

    explicit Channels(size_t chId) {
        if (chId < MaxChannels) {
            activeMask = (static_cast<StorageType>(1) << chId);
        }
    }

    explicit Channels(Enum ch) : Channels(static_cast<size_t>(ch)) {}

    Channels(std::initializer_list<size_t> channels) {
        for (size_t chId : channels) {
            if (chId < MaxChannels) {
                activeMask |= (static_cast<StorageType>(1) << chId);
            }
        }
    }

    Channels(std::initializer_list<Enum> channels) {
        for (Enum ch : channels) {
            size_t chId = static_cast<size_t>(ch);
            if (chId < MaxChannels) {
                activeMask |= (static_cast<StorageType>(1) << chId);
            }
        }
    }

    explicit Channels(StorageType mask) : activeMask(mask & getValidMask()) {}

    void enable(size_t chId) {
        if (chId < MaxChannels) {
            activeMask |= (static_cast<StorageType>(1) << chId);
        }
    }

    void enable(Enum ch) { enable(static_cast<size_t>(ch)); }

    void disable(size_t chId) {
        if (chId < MaxChannels) {
            activeMask &= ~(static_cast<StorageType>(1) << chId);
        }
    }

    void disable(Enum ch) { disable(static_cast<size_t>(ch)); }

    bool isEnabled(size_t chId) const {
        if (chId < MaxChannels) {
            return (activeMask & (static_cast<StorageType>(1) << chId)) != 0;
        }
        return false;
    }

    bool isEnabled(Enum ch) const { return isEnabled(static_cast<size_t>(ch)); }

    struct Iterator {
        StorageType mask;
        bool operator!=(const Iterator& other) const { return mask != other.mask; }
        int operator*() const { return std::countr_zero(mask); }
        Iterator& operator++() {
            mask &= (mask - 1);
            return *this;
        }
    };

    size_t count() const { return std::popcount(activeMask); }

    void enableAll() { activeMask = getValidMask(); }

    void reset() { activeMask = 0; }

    Iterator begin() const { return Iterator{activeMask}; }
    Iterator end() const { return Iterator{0}; }

    constexpr StorageType getValidMask() const {
        return (MaxChannels == 8 * sizeof(StorageType)) ? ~static_cast<StorageType>(0) : (static_cast<StorageType>(1) << MaxChannels) - 1;
    }

    Channels& operator=(const Channels& other) {
        if (this != &other) {
            activeMask = other.activeMask;
        }
        return *this;
    }

    Channels& operator=(Channels&& other) noexcept {
        if (this != &other) {
            activeMask = other.activeMask;
            other.activeMask = 0;
        }
        return *this;
    }

    Channels operator+(const Channels& other) const {
        Channels result;
        result.activeMask = activeMask | other.activeMask;
        return result;
    }

    Channels& operator+=(const Channels& other) {
        activeMask |= other.activeMask;
        return *this;
    }

    Channels operator-(const Channels& other) const {
        Channels result;
        result.activeMask = activeMask & ~other.activeMask;
        return result;
    }

    Channels& operator-=(const Channels& other) {
        activeMask &= ~other.activeMask;
        return *this;
    }

    Channels operator&(const Channels& other) const {
        Channels result;
        result.activeMask = activeMask & other.activeMask;
        return result;
    }

    Channels& operator&=(const Channels& other) {
        activeMask &= other.activeMask;
        return *this;
    }

    Channels operator|(const Channels& other) const { return (*this + other); }

    Channels& operator|=(const Channels& other) { return (*this += other); }

    Channels operator^(const Channels& other) const {
        Channels result;
        result.activeMask = activeMask ^ other.activeMask;
        return result;
    }

    Channels& operator^=(const Channels& other) {
        activeMask ^= other.activeMask;
        return *this;
    }

    Channels operator~() const {
        Channels result;
        StorageType mask = (MaxChannels == 8 * sizeof(StorageType)) ? ~static_cast<StorageType>(0) : (static_cast<StorageType>(1) << MaxChannels) - 1;
        result.activeMask = (~activeMask) & mask;
        return result;
    }

    bool operator==(const Channels& other) const { return activeMask == other.activeMask; }

    bool operator!=(const Channels& other) const { return !(*this == other); }

    bool operator[](size_t chId) const { return isEnabled(chId); }

    bool operator[](Enum ch) const { return isEnabled(ch); }

    struct ChannelProxy {
        Channels& manager;
        size_t channelId;

        operator bool() const { return manager.isEnabled(channelId); }

        ChannelProxy& operator=(bool value) {
            if (value)
                manager.enable(channelId);
            else
                manager.disable(channelId);
            return *this;
        }
    };

    ChannelProxy operator[](size_t chId) { return ChannelProxy{*this, chId}; }

    ChannelProxy operator[](Enum ch) { return ChannelProxy{*this, static_cast<size_t>(ch)}; }

	auto toString() const -> std::string { return std::to_string(activeMask); }

	static auto fromString(const std::string &str) -> Channels
	{
		Channels result;
		try {
			StorageType mask = static_cast<StorageType>(std::stoull(str));
			result.activeMask = mask & result.getValidMask();
		} catch (...) {
			result.activeMask = 0;
		}
		return result;
	}

	friend std::ostream& operator<<(std::ostream& os, const Channels& cm) {
        os << "Channels<" << typeid(Enum).name() << "> [";
        bool first = true;
        for (int ch : cm) {
            if (!first)
                os << ", ";
            os << ch << "(" << static_cast<Enum>(ch) << ")";
            first = false;
        }
        os << "]";
        return os;
    }

	auto format() -> std::string
	{
		if (activeMask == 0) {
			return "";
		}

		std::string result;
		bool first = true;

		for (auto it = begin(); it != end(); ++it) {
			if (!first) {
				result += ", ";
			}
			result += "CH" + std::to_string(*it + 1);
			first = false;
		}

		return result;
	}

	friend std::istream &operator>>(std::istream &is, Channels &cm)
	{
		size_t chId;
		if (is >> chId) {
            cm.enable(chId);
        }
        return is;
	}

	Channels &operator+=(Enum ch)
	{
		enable(ch);
        return *this;
	}

	Channels& operator-=(Enum ch) {
        disable(ch);
        return *this;
    }

    Channels operator+(Enum ch) const {
        Channels result = *this;
        result += ch;
        return result;
    }

    Channels operator-(Enum ch) const {
        Channels result = *this;
        result -= ch;
        return result;
    }

    bool operator&(Enum ch) const { return isEnabled(ch); }
};

typedef Channels<DACChannels, MAX_DAC_CHANNELS> dac_channels_t;
typedef Channels<ADCChannels, MAX_ADC_CHANNELS> adc_channels_t;

#endif
