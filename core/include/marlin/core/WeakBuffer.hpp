#ifndef MARLIN_CORE_WEAKBUFFER_HPP
#define MARLIN_CORE_WEAKBUFFER_HPP

#include <stdint.h>
#include <uv.h>
#include <memory>
#include <utility>
#include <optional>
// DONOT REMOVE. FAILS TO COMPILE ON MAC OTHERWISE
#include <array>

namespace marlin {
namespace core {

/// @brief Byte buffer implementation with modifiable bounds
/// @headerfile WeakBuffer.hpp <marlin/core/WeakBuffer.hpp>
class WeakBuffer {
protected:
	/// Pointer to underlying memory
	uint8_t *buf;
	/// Capacity of memory
	size_t capacity;
	/// Start index in memory, inclusive
	size_t start_index;
	/// End index in memory, non-inclusive
	size_t end_index;

public:
	/// @brief Construct from uint8_t array
	/// @param buf Pointer to bytes
	/// @param size Size of bytes
	WeakBuffer(uint8_t *const buf, size_t const size);

	/// Move contructor
	WeakBuffer(WeakBuffer &&b) = default;

	/// Copy contructor
	WeakBuffer(WeakBuffer const &b) = default;

	/// Move assign
	WeakBuffer &operator=(WeakBuffer &&b) = default;

	/// Copy assign
	WeakBuffer &operator=(WeakBuffer const &b) = default;

	/// Start of buffer
	inline uint8_t *data() const {
		return buf + start_index;
	}

	/// Length of buffer
	inline size_t size() const {
		return end_index - start_index;
	}

	/// @name Bounds change
	/// @{

	//! Moves start of buffer forward and covers given number of bytes
	[[nodiscard]] bool cover(size_t const num);
	/// Moves start of buffer forward and covers given number of bytes without bounds checking
	void cover_unsafe(size_t const num);

	/// Moves start of buffer backward and uncovers given number of bytes
	[[nodiscard]] bool uncover(size_t const num);
	/// Moves start of buffer backward and uncovers given number of bytes without bounds checking
	void uncover_unsafe(size_t const num);

	/// Moves end of buffer backward and covers given number of bytes
	[[nodiscard]] bool truncate(size_t const num);
	/// Moves end of buffer backward and covers given number of bytes without bounds checking
	void truncate_unsafe(size_t const num);

	/// Moves end of buffer forward and uncovers given number of bytes
	[[nodiscard]] bool expand(size_t const num);
	/// Moves end of buffer forward and uncovers given number of bytes without bounds checking
	void expand_unsafe(size_t const num);
	/// @}

	/// @name Read/Write arbitrary data
	/// @{
	//-------- Arbitrary reads begin --------//

	/// Read arbitrary data starting at given byte
	[[nodiscard]] bool read(size_t const pos, uint8_t* const out, size_t const size) const;
	/// Read arbitrary data starting at given byte without bounds checking
	void read_unsafe(size_t const pos, uint8_t* const out, size_t const size) const;

	//-------- Arbitrary reads end --------//

	//-------- Arbitrary writes begin --------//

	/// Write arbitrary data starting at given byte
	[[nodiscard]] bool write(size_t const pos, uint8_t const* const in, size_t const size);
	/// Write arbitrary data starting at given byte without bounds checking
	void write_unsafe(size_t const pos, uint8_t const* const in, size_t const size);

	//-------- Arbitrary writes end --------//
	/// @}

	/// @name Read/Write uint8_t
	/// @{
	//-------- 8 bit reads begin --------//

	/// Read uint8_t starting at given byte
	std::optional<uint8_t> read_uint8(size_t const pos) const;
	/// Read uint8_t starting at given byte without bounds checking
	uint8_t read_uint8_unsafe(size_t const pos) const;

	//-------- 8 bit reads end --------//


	//-------- 8 bit writes begin --------//

	/// Write uint8_t starting at given byte
	[[nodiscard]] bool write_uint8(size_t const pos, uint8_t const num);
	/// Write uint8_t starting at given byte without bounds checking
	void write_uint8_unsafe(size_t const pos, uint8_t const num);

	//-------- 8 bit writes end --------//
	/// @}

	/// @name Read/Write uint16_t
	/// @{
	//-------- 16 bit reads begin --------//

	/// Read uint16_t starting at given byte
	std::optional<uint16_t> read_uint16(size_t const pos) const;
	/// Read uint16_t starting at given byte without bounds checking
	uint16_t read_uint16_unsafe(size_t const pos) const;

	/// Read uint16_t starting at given byte,
	/// converting from LE to host endian
	std::optional<uint16_t> read_uint16_le(size_t const pos) const;

	/// Read uint16_t starting at given byte without bounds checking,
	/// converting from LE to host endian
	uint16_t read_uint16_le_unsafe(size_t const pos) const;

	/// Read uint16_t starting at given byte,
	/// converting from BE to host endian
	std::optional<uint16_t> read_uint16_be(size_t const pos) const;
	/// Read uint16_t starting at given byte without bounds checking,
	/// converting from BE to host endian
	uint16_t read_uint16_be_unsafe(size_t const pos) const;

	//-------- 16 bit reads end --------//

	//-------- 16 bit writes begin --------//

	/// Write uint16_t starting at given byte
	[[nodiscard]] bool write_uint16(size_t const pos, uint16_t const num);
	/// Write uint16_t starting at given byte without bounds checking
	void write_uint16_unsafe(size_t const pos, uint16_t const num);

	/// Write uint16_t starting at given byte,
	/// converting from host endian to LE
	[[nodiscard]] bool write_uint16_le(size_t const pos, uint16_t const num);
	/// Write uint16_t starting at given byte without bounds checking,
	/// converting from host endian to LE
	void write_uint16_le_unsafe(size_t const pos, uint16_t const num);

	/// Write uint16_t starting at given byte,
	/// converting from host endian to BE
	[[nodiscard]] bool write_uint16_be(size_t const pos, uint16_t const num);
	/// Write uint16_t starting at given byte without bounds checking,
	/// converting from host endian to BE
	void write_uint16_be_unsafe(size_t const pos, uint16_t const num);

	//-------- 16 bit writes end --------//
	/// @}

	/// @name Read/Write uint32_t
	/// @{
	//-------- 32 bit reads begin --------//

	/// Read uint32_t starting at given byte
	std::optional<uint32_t> read_uint32(size_t const pos) const;
	/// Read uint32_t starting at given byte without bounds checking
	uint32_t read_uint32_unsafe(size_t const pos) const;

	/// Read uint32_t starting at given byte,
	/// converting from LE to host endian
	std::optional<uint32_t> read_uint32_le(size_t const pos) const;
	/// Read uint32_t starting at given byte without bounds checking,
	/// converting from LE to host endian
	uint32_t read_uint32_le_unsafe(size_t const pos) const;

	/// Read uint32_t starting at given byte,
	/// converting from BE to host endian
	std::optional<uint32_t> read_uint32_be(size_t const pos) const;
	/// Read uint32_t starting at given byte without bounds checking,
	/// converting from BE to host endian
	uint32_t read_uint32_be_unsafe(size_t const pos) const;

	//-------- 32 bit reads end --------//

	//-------- 32 bit writes begin --------//

	/// Write uint32_t starting at given byte
	[[nodiscard]] bool write_uint32(size_t const pos, uint32_t const num);
	/// Write uint32_t starting at given byte without bounds checking
	void write_uint32_unsafe(size_t const pos, uint32_t const num);

	/// Write uint32_t starting at given byte,
	/// converting from host endian to LE
	[[nodiscard]] bool write_uint32_le(size_t const pos, uint32_t const num);
	/// Write uint32_t starting at given byte without bounds checking,
	/// converting from host endian to LE
	void write_uint32_le_unsafe(size_t const pos, uint32_t const num);

	/// Write uint32_t starting at given byte,
	/// converting from host endian to BE
	[[nodiscard]] bool write_uint32_be(size_t const pos, uint32_t const num);
	/// Write uint32_t starting at given byte without bounds checking,
	/// converting from host endian to BE
	void write_uint32_be_unsafe(size_t const pos, uint32_t const num);

	//-------- 32 bit writes end --------//
	/// @}

	/// @name Read/Write uint64_t
	/// @{
	//-------- 64 bit reads begin --------//

	/// Read uint64_t starting at given byte
	std::optional<uint64_t> read_uint64(size_t const pos) const;
	/// Read uint64_t starting at given byte without bounds checking
	uint64_t read_uint64_unsafe(size_t const pos) const;

	/// Read uint64_t starting at given byte,
	/// converting from LE to host endian
	std::optional<uint64_t> read_uint64_le(size_t const pos) const;
	/// Read uint64_t starting at given byte without bounds checking,
	/// converting from LE to host endian
	uint64_t read_uint64_le_unsafe(size_t const pos) const;

	/// Read uint64_t starting at given byte,
	/// converting from BE to host endian
	std::optional<uint64_t> read_uint64_be(size_t const pos) const;
	/// Read uint64_t starting at given byte without bounds checking,
	/// converting from BE to host endian
	uint64_t read_uint64_be_unsafe(size_t const pos) const;

	//-------- 64 bit reads end --------//

	//-------- 64 bit writes begin --------//

	/// Write uint64_t starting at given byte
	[[nodiscard]] bool write_uint64(size_t const pos, uint64_t const num);
	/// Write uint64_t starting at given byte without bounds checking
	void write_uint64_unsafe(size_t const pos, uint64_t const num);

	/// Write uint64_t starting at given byte,
	/// converting from host endian to LE
	[[nodiscard]] bool write_uint64_le(size_t const pos, uint64_t const num);
	/// Write uint64_t starting at given byte without bounds checking,
	/// converting from host endian to LE
	void write_uint64_le_unsafe(size_t const pos, uint64_t const num);

	/// Write uint64_t starting at given byte,
	/// converting from host endian to BE
	[[nodiscard]] bool write_uint64_be(size_t const pos, uint64_t const num);
	/// Write uint64_t starting at given byte without bounds checking,
	/// converting from host endian to BE
	void write_uint64_be_unsafe(size_t const pos, uint64_t const num);

	//-------- 64 bit writes end --------//
	/// @}
};

} // namespace core
} // namespace marlin

#endif // MARLIN_CORE_WEAKBUFFER_HPP
