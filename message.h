#pragma once
#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>

/**
 * \brief Message structure
 */
struct message {
	uint64_t bloom[8];    /**< Bloom filter */
	uint64_t address[2];  /**< Address */
	uint64_t timestamp;   /**< Timestamp */
	uint64_t bandwidth;   /**< Bandwidth */
	uint32_t latency;     /**< Average packet delay */
	uint16_t variance;    /**< Packet delay variance */
	uint16_t reliability; /**< Reliability */
	uint16_t pmtu;        /**< Path MTU */
	uint8_t  prefix;      /**< Prefix length */
	uint8_t  hops;        /**< Hop count */
	uint32_t padding;     /**< Padding */
} __packed;

#endif
