/*
 * Copyright (C) 2004, 2006-2011 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_ECONOMY_INPUT_QUEUE_H
#define WL_ECONOMY_INPUT_QUEUE_H

#include "logic/map_objects/immovable.h"
#include "logic/map_objects/tribes/wareworker.h"
#include "logic/widelands.h"

namespace Widelands {

class Economy;
class Game;
class MapObjectLoader;
struct MapObjectSaver;
class Player;
class Request;

/**
 * Base class for input queues of wares and workers.
 * Mentioning "wares" in the following nearly always means "wares or workers".
 * @todo Add builder-queue to expeditions?
 * @todo Create regression tests (for WorkersQueue)
 */
class InputQueue {
public:

	/**
	 * Destructor.
	 * Does nothing currently.
	 */
	virtual ~InputQueue() { }

	/**
	 * The declaration of a callback function which can be registered to get notified
	 * when wares or workers arrive at the building and should be added to the queue.
	 * @param game The game the queue is part of.
	 * @param q The \c InputQueue the ware or worker should be added to.
	 * @param ware The index of the ware which arrived, if the queue is a WaresQueue.
	 * @param worker The worker which arrived, if the queue is a WorkersQueue.
	 * @param data Unspecified data which has been given when calling set_callback().
	 */
	using CallbackFn = void(Game& g, InputQueue* q, DescriptionIndex ware, Worker* worker, void* data);

	/**
	 * Returns the index of the ware or worker which is handled by the queue.
	 * @param The DescriptionIndex of whatever is stored here.
	 */
	DescriptionIndex get_index() const {
		return index_;
	}

	/**
     * Get the maximum amount of wares or workers which should be stored here.
     * This is a value which can be influenced by the player with the provided buttons.
     * @return The maximum number of wares or workers which should be here.
	 */
	Quantity get_max_fill() const {
		return max_fill_;
	}

	/**
	 * Whether wares or workers are stored in this queue.
	 * @return Whether wares or workers are stored in this queue.
	 */
	WareWorker get_type() const {
		return type_;
	}

	/**
	 * The maximum size of the queue as defined by the building.
	 * @return The maximum size.
	 */
	Quantity get_max_size() const {
		return max_size_;
	}

	/**
	 * The amount of wares or workers which are stored here currently.
	 * This might temporarily be larger than get_max_fill() but will
	 * be smaller than get_max_size().
	 * @return The amount at this moment.
	 */
	virtual Quantity get_filled() const = 0;

	/**
	 * Clear the queue appropriately.
	 * Implementing classes should call update() at last to remove the request.
	 */
	virtual void cleanup() = 0;

	/**
	 * Set the callback function that is called when an item has arrived.
	 * @param fn The function to call.
	 * @param data Unspecified data which will be passed on calls of \c fn.
	 */
	void set_callback(CallbackFn* fn, void* data);

	/**
	 * Remove the wares in this queue from the given economy (used in accounting).
	 * Implementing classes have to set the economy of the potential request.
	 * @param e The economy to remove the wares or workers from.
	 */
	virtual void remove_from_economy(Economy& e) = 0;

	/**
	 * Add the wares in this queue to the given economy (used in accounting).
	 * Implementing classes have to set the economy of the potential request.
	 * @param The economy to add the wares or workers to.
	 */
	virtual void add_to_economy(Economy& e) = 0;

	/**
	 * Change size of the queue.
	 * This influences how many wares can be in here at maximum.
	 * @param q The new maximum size.
	 */
	void set_max_size(Quantity q);

	/**
	 * Change the number of wares that should be available in this queue.
	 *
	 * This is similar as setting the maximum size,
	 * but if there are more wares than that in the queue, they will not get
	 * lost (the building should drop them). This is the method called when the player
	 * pressed the buttons in the gui.
	 * @param q The maximum number of wares which should be stored here.
	 */
	virtual void set_max_fill(Quantity q);

	/**
	 * Change fill status of the queue. This creates or removes wares as required.
	 * Note that the wares are created out of thin air respectively are removed without
	 * dropping them on the street.
	 * @param q The number of wares which are stored here.
	 */
	virtual void set_filled(Quantity q) = 0;

	/**
	 * Set the time between consumption of items when the owning building
	 * is consuming at full speed.
	 *
	 * This interval is merely a hint for the Supply/Request balancing code.
	 * @param i The interval in ms.
	*/
	void set_consume_interval(uint32_t i);

	/**
	 * Returns the player owning the building containing this queue.
	 * @return A reference to the owning player.
	 */
	Player& owner() const {
		return owner_.owner();
	}

	/**
	 * Overwrites the state of this class with the read data.
	 * @param fr A stream to read the data from.
	 * @param game The game this queue will be part of.
	 * @param mol Don't know. Required to pass to Request::read().
	 */
	void read(FileRead& f, Game& g, MapObjectLoader& mol);

	/**
	 * Writes the state of this class.
	 * @param fw A stream to write the data to.
	 * @param game The game this queue is part of.
	 * @param mos Don't know. Required to pass to Request::write().
	 */
	void write(FileWrite& w, Game& g, MapObjectSaver& s);

protected:
	/**
	 * Pre-initialize a InputQueue.
	 * @param init_owner The building the queue is part of.
	 * @param init_index The index of the ware or worker that will be stored.
	 * @param init_max_size The maximum amount that can be stored.
	 * @param init_type Whether wares or workers are stored in this queue.
	*/
	InputQueue(PlayerImmovable&, DescriptionIndex, uint8_t size, WareWorker);

	/**
	 * Called when an item arrives at the owning building.
	 * Most likely only one of \c i or \c w will be valid.
	 * @param g The game the queue is part of.
	 * @param r The request for the ware or worker.
	 * @param i The index of the arrived ware or worker.
	 * @param w The arrived worker or \c nullptr.
	 * @param b The building where the ware or worker arrived at.
	 */
	static void request_callback(Game& g, Request& r, DescriptionIndex i, Worker* w, PlayerImmovable& b);

	/**
	 * Updates the request.
	 * You must call this after every call to set_*().
	*/
	void update();

	/**
	 * Called when an item arrives at the owning building.
	 * Most likely only one of \c i or \c w will be valid.
	 * @param index The index of the arrived ware in case of ware queues.
	 *          Not sure about its value in worker queues.
	 * @param worker The arrived worker or \c nullptr.
	 */
	virtual void entered(DescriptionIndex index, Worker* worker) = 0;

	/**
	 * Overwrites the state of the subclass with the read data.
	 * @param fr A stream to read the data from.
	 * @param game The game this queue will be part of.
	 * @param mol Don't know.
	 */
	virtual void read_child(FileRead& f, Game& g, MapObjectLoader& mol) = 0;

	/**
	 * Writes the state of the subclass.
	 * @param fw A stream to write the data to.
	 * @param game The game this queue is part of.
	 * @param mos Don't know.
	 */
	virtual void write_child(FileWrite& w, Game& g, MapObjectSaver& s) = 0;

	/// The building this queue is part of.
	PlayerImmovable& owner_;
	/// ID of stored whatever.
	DescriptionIndex index_;
	/// Amount that fit into the queue maximum.
	Quantity max_size_;
	/// Amount that should be ideally in this queue.
	Quantity max_fill_;

	/// Whether wares or workers are stored in the queue.
	const WareWorker type_;

	/// Time in ms between consumption at full speed.
	uint32_t consume_interval_;

	/// The currently pending request.
	// NOCOM(#codereview): this should probaby be a unique ptr
	Request* request_;

	/// The function to call on fulfilled request.
	CallbackFn* callback_fn_;
	/// Undefined data to pass to function.
	void* callback_data_;
};
}

#endif  // end of include guard: WL_ECONOMY_INPUT_QUEUE_H
