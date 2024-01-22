#pragma once
#include <iterator>
#include <memory>
#include <list>
#include <algorithm>
#include <exception>
#include <compare>
#include <iostream>


namespace fefu_laboratory_two {
	template <typename T>
	class Allocator {
	public:
		using value_type = T;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using pointer = T*;
		using const_pointer = const T*;
		using reference = T&;
		using const_reference = const T&;


		constexpr Allocator() noexcept {};

		constexpr Allocator(const Allocator& other) noexcept = default;

		template <class U>
		constexpr Allocator(const Allocator<U>& other) noexcept {};

		~Allocator() = default;

		pointer allocate(size_type N) {
			pointer ptr = static_cast<pointer>(::operator new(sizeof(value_type) * N));
			if (ptr != nullptr) {
				return ptr;
			}
			else {
				throw std::bad_alloc();
			}
		}

		void deallocate(pointer p, const size_t N) noexcept {
			static_cast<void>(N);
			free(p);
		}
	};

	template <typename ValueType, typename Allocator = Allocator<ValueType>>
	class Chunk {
	public:
		using size_type = std::size_t;
		ValueType* list = nullptr;
		Chunk* prev = nullptr;
		Chunk* next = nullptr;
		int chunk_size = 0;
		int num_of_elements = 0;
		Allocator allocator;

		Chunk(int N) {
			list = allocator.allocate(N);
			chunk_size = N;
		}

		ValueType* get_data() {
			ValueType* data = allocator.allocate(chunk_size);
			for (int i = 0; i < chunk_size; i++)
				data[i] = list[i];
			return data;
		}

		ValueType* begin() {
			return list;
		}

		ValueType* end() {
			return list + num_of_elements;
		}

		void resize(size_type new_size) {
			if (new_size < 0)
				throw std::invalid_argument("New size cannot be negative");

			if (new_size == chunk_size)
				return;

			size_type min_v = (chunk_size > new_size) ? new_size : chunk_size;
			ValueType* new_list = allocator.allocate(new_size);
			for (int i = 0; i < min_v; ++i) {
				new_list[i] = list[i];
			}

			if (num_of_elements > new_size)
				num_of_elements = new_size;

			allocator.deallocate(list, chunk_size);
			list = new_list;
			chunk_size = new_size;
		}

		void resize(size_type new_size, const ValueType& value) {
			if (new_size < 0)
				throw std::invalid_argument("New size cannot be negative");

			if (new_size == chunk_size)
				return;
			
			ValueType* new_list = allocator.allocate(new_size);
			if (new_size <= chunk_size) {
				for (int i = 0; i < new_size; ++i) {
					new_list[i] = list[i];
				}
			}
			else {
				for (int i = 0; i < chunk_size; ++i) {
					new_list[i] = list[i];
				}

				for (int i = chunk_size; i < new_size; ++i) {
					new_list[i] = value;
				}
			}


			if (num_of_elements > new_size)
				num_of_elements = new_size;

			allocator.deallocate(list, chunk_size);
			list = new_list;
			chunk_size = new_size;
			num_of_elements = new_size;
		}
	};

	template<typename ValueType>
	class ChunkListInterface {
	public:
		virtual ~ChunkListInterface() {}
		virtual ValueType& at(size_t index) = 0;
		virtual const ValueType& at(size_t index) const = 0;
		virtual size_t size() const noexcept = 0;
		virtual ValueType& operator[](std::ptrdiff_t n) = 0;
		virtual const ValueType& operator[](std::ptrdiff_t n) const = 0;
	};

	template <typename ValueType>
	class ChunkList_iterator {
	protected:
		int elem_index = 0;
		ChunkListInterface<ValueType>* list = nullptr;
		ValueType* current_value = nullptr;
	public:
		using iterator_category = std::random_access_iterator_tag;
		using value_type = ValueType;
		using difference_type = std::ptrdiff_t;
		using pointer = ValueType*;
		using reference = ValueType&;

		int get_index() { return elem_index; };

		constexpr ChunkList_iterator() noexcept = default;

		ChunkList_iterator(ChunkListInterface<ValueType>* chunk, int index, ValueType* value) :
			list(chunk),
			elem_index(index),
			current_value(value)
		{
		};

		ChunkList_iterator(const ChunkList_iterator<ValueType>& other)
		{
			list = other.list;
			elem_index = other.elem_index;
			current_value = other.current_value;
		};

		ChunkList_iterator& operator=(const ChunkList_iterator& other) {
			list = other.list;
			elem_index = other.elem_index;
			current_value = other.current_value;
		};

		~ChunkList_iterator() = default;

		void swap(ChunkList_iterator<ValueType>& a, ChunkList_iterator<ValueType>& b) {
			std::swap(a.list, b.list);
			std::swap(a.current_value, b.current_value);
			std::swap(a.elem_index, b.elem_index);
		};

		friend bool operator==(const ChunkList_iterator<ValueType>& lhs,
			const ChunkList_iterator<ValueType>& rhs) {
			return lhs.current_value == rhs.current_value;
		};

		friend bool operator!=(const ChunkList_iterator<ValueType>& lhs,
			const ChunkList_iterator<ValueType>& rhs) {
			return !(lhs.current_value == rhs.current_value);
		};

		reference operator*() const { return *current_value; };
		pointer operator->() const { return current_value; };

		ChunkList_iterator operator++(int) {
			if (elem_index + 1 == list->size())
				return ChunkList_iterator();
			elem_index++;
			current_value = &list->at(elem_index);
			return *this;
		};
		ChunkList_iterator operator--(int) {
			elem_index--;
			if (elem_index <= -1)
				return *this;

			current_value = &list->at(elem_index);
			return *this;
		};

		ChunkList_iterator& operator++() {
			if (elem_index + 1 == list->size()) {
				current_value = nullptr;
				list = nullptr;
				elem_index = 0;
				return *this;
			}
			elem_index++;
			current_value = &list->at(elem_index);
			return *this;
		};

		ChunkList_iterator& operator--() {
			elem_index--;
			if (elem_index <= -1)
				return *this;

			current_value = &list->at(elem_index);
			return *this;
		};

		ChunkList_iterator operator+(const difference_type& n) const {
			return ChunkList_iterator(list, elem_index + n, &list->at(elem_index + n));
		};


		ChunkList_iterator operator-(const difference_type& n) const {
			return ChunkList_iterator(list, elem_index - n, &list->at(elem_index - n));
		};

		ChunkList_iterator& operator+=(const difference_type& n) {
			elem_index += n;
			current_value = &list->at(elem_index);
			return *this;
		};

		ChunkList_iterator& operator-=(const difference_type& n) {
			elem_index -= n;
			current_value = &list->at(elem_index);
			return *this;
		};


		reference operator[](const difference_type& n) {
			return list[n];
		};

		friend bool operator<(const ChunkList_iterator<ValueType>& lhs,
			const ChunkList_iterator<ValueType>& rhs) {
			return lhs.elem_index < rhs.elem_index;
		};
		friend bool operator<=(const ChunkList_iterator<ValueType>& lhs,
			const ChunkList_iterator<ValueType>& rhs) {
			return lhs.elem_index <= rhs.elem_index;
		};
		friend bool operator>(const ChunkList_iterator<ValueType>& lhs,
			const ChunkList_iterator<ValueType>& rhs) {
			return lhs.elem_index > rhs.elem_index;
		};
		friend bool operator>=(const ChunkList_iterator<ValueType>& lhs,
			const ChunkList_iterator<ValueType>& rhs) {
			return lhs.elem_index >= rhs.elem_index;
		};
	};

	template <typename ValueType>
	class ChunkList_const_iterator {
	public:
		int elem_index = 0;
		const ChunkListInterface<ValueType>* list = nullptr;
		const ValueType* current_value = nullptr;

		using iterator_category = std::random_access_iterator_tag;
		using value_type = ValueType;
		using difference_type = std::ptrdiff_t;
		using const_pointer = const ValueType*;
		using const_reference = const ValueType&;

		const int get_index() const { return elem_index; };

		ChunkList_iterator<ValueType> constIteratorToIterator() {
			return ChunkList_iterator<ValueType>(const_cast<ChunkListInterface<ValueType>*>(list), elem_index, const_cast<ValueType*>(current_value));
		}

		constexpr ChunkList_const_iterator() noexcept = default;

		ChunkList_const_iterator(ChunkListInterface<ValueType>* chunk, int index, ValueType* value) :
			list(chunk),
			elem_index(index),
			current_value(value) 
		{
		};

		ChunkList_const_iterator(const ChunkListInterface<ValueType>* chunk, int index, const ValueType* value) :
			list(chunk),
			elem_index(index),
			current_value(value) 
		{	
		};

		ChunkList_const_iterator(const ChunkList_const_iterator<ValueType>& other) {
			list = other.list;
			elem_index = other.elem_index;
			current_value = other.current_value;
		};

		ChunkList_const_iterator& operator=(const ChunkList_const_iterator&) = default;

		~ChunkList_const_iterator() = default;

		void swap(ChunkList_const_iterator<ValueType>& a, ChunkList_const_iterator<ValueType>& b) {
			std::swap(a.list, b.list);
			std::swap(a.current_value, b.current_value);
			std::swap(a.elem_index, b.elem_index);
		};

		friend bool operator==(const ChunkList_const_iterator<ValueType>& lhs,
			const ChunkList_const_iterator<ValueType>& rhs) {
			return lhs.current_value == rhs.current_value;
		};
		friend bool operator!=(const ChunkList_const_iterator<ValueType>& lhs,
			const ChunkList_const_iterator<ValueType>& rhs) {
			return lhs.current_value != rhs.current_value;
		};

		const_reference operator*() const { return *current_value; };
		const_pointer operator->() const { return current_value; };
		const_reference operator[](const difference_type& n) {
			return list[n];
		};

		ChunkList_const_iterator operator++(int) {
			if (elem_index + 1 == list->size())
				return ChunkList_const_iterator();
			elem_index++;
			current_value = &list->at(elem_index);
			return *this;
		};

		ChunkList_const_iterator operator--(int) {
			return ChunkList_iterator<ValueType>::operator--(0);
		};

		ChunkList_const_iterator& operator++() {
			if (elem_index + 1 == list->size()) {
				current_value = nullptr;
				list = nullptr;
				elem_index = 0;
				return *this;
			}
			elem_index++;
			current_value = &list->at(elem_index);
			return *this;
		};

		ChunkList_const_iterator& operator--() {
			if (elem_index - 1 == -1)
				throw std::exception();
			elem_index--;
			current_value = &list->at(elem_index);
			return *this;
		};

		ChunkList_const_iterator operator+(const difference_type& n) const {
			return ChunkList_const_iterator(list, elem_index + n, &list->at(elem_index + n));
		};


		ChunkList_const_iterator operator-(const difference_type& n) const {
			return ChunkList_const_iterator(list, elem_index - n, &list->at(elem_index - n));
		};

		ChunkList_const_iterator& operator+=(const difference_type& n) {
			elem_index += n;
			current_value = &list->at(elem_index);
			return *this;
		};

		ChunkList_const_iterator& operator-=(const difference_type& n) {
			elem_index -= n;
			current_value = &list->at(elem_index);
			return *this;
		};

		friend bool operator<(const ChunkList_const_iterator<ValueType>& lhs,
			const ChunkList_const_iterator<ValueType>& rhs) {
			return lhs.elem_index < rhs.elem_index;
		};
		friend bool operator<=(const ChunkList_const_iterator<ValueType>& lhs,
			const ChunkList_const_iterator<ValueType>& rhs) {
			return lhs.elem_index <= rhs.elem_index;
		};
		friend bool operator>(const ChunkList_const_iterator<ValueType>& lhs,
			const ChunkList_const_iterator<ValueType>& rhs) {
			return lhs.elem_index > rhs.elem_index;
		};
		friend bool operator>=(const ChunkList_const_iterator<ValueType>& lhs,
			const ChunkList_const_iterator<ValueType>& rhs) {
			return lhs.elem_index >= rhs.elem_index;
		};
	};

	template <typename T, int N, typename Allocator = Allocator<T>>
	class ChunkList : public ChunkListInterface<T> {
	protected:
		Chunk<T, Allocator>* first_chunk = nullptr;
		int list_size = 0;
		int chunk_size = N;
	public:

		using value_type = T;
		using allocator_type = Allocator;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = typename std::allocator_traits<Allocator>::pointer;
		using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
		using iterator = ChunkList_iterator<value_type>;
		using const_iterator = ChunkList_const_iterator<value_type>;

		/// @brief Default constructor. Constructs an empty container with a
		/// default-constructed allocator.
		ChunkList() : first_chunk(new Chunk<value_type>(N)) {};

		/// @brief Constructs an empty container with the given allocator
		/// @param alloc allocator to use for all memory allocations of this container
		/// s
		/*explicit ChunkList(const Allocator& alloc);*/

		/// @brief Constructs the container with count copies of elements with value
		/// and with the given allocator
		/// @param count the size of the container
		/// @param value the value to initialize elements of the container with
		/// @param alloc allocator to use for all memory allocations of this container
		ChunkList(size_type count, const T& value = T(), const Allocator& alloc = Allocator())
			: first_chunk(new Chunk<value_type, allocator_type>(N))
		{
			int i = 0;
			auto current_chunk = first_chunk;
			while (i < count) {
				current_chunk->allocator = alloc;
				for (int j = 0; j < N; j++) {
					current_chunk->list[j] = value;
					i++;
					if (i == count) {
						break;
					}
				}
				if (i < count) {
					current_chunk->next = new Chunk<value_type, allocator_type>(N);
					current_chunk = current_chunk->next;
				}
			}
		};

		/// @brief Constructs the container with count default-inserted instances of
		/// T. No copies are made.
		/// @param count the size of the container
		/// @param alloc allocator to use for all memory allocations of this container
		explicit ChunkList(size_type count, const Allocator& alloc = Allocator())
			: first_chunk(new Chunk<value_type, allocator_type>(N))
		{
			int i = 0;
			Chunk<value_type, allocator_type>* current_chunk = first_chunk;
			while (i < count) {
				current_chunk->allocator = alloc;
				for (int j = 0; j < N; j++) {
					current_chunk->list[j] = T();
					i++;
					if (i == count)
						break;
				}
				if (i < count) {
					current_chunk->next = new Chunk<value_type, allocator_type>(N);
					current_chunk = current_chunk->next;
				}
			}
		};

		/// @brief Constructs the container with the contents of the range [first,
		/// last).
		/// @tparam InputIt Input Iterator
		/// @param first, last 	the range to copy the elements from
		/// @param alloc allocator to use for all memory allocations of this container
		template <class InputIt>
		ChunkList(InputIt first, InputIt last, const Allocator& alloc = Allocator()) 
			: first_chunk(new Chunk<value_type, allocator_type>(N))
		{
			Chunk<value_type, allocator_type>* current_chunk = first_chunk;
			auto it = first;
			int i = 0;
			while (it != last) {
				current_chunk->allocator = alloc;	
				for (i = 0; i < N && it != last; ++i, ++it) {
					current_chunk->list[i] = *it;
					current_chunk->num_of_elements++;
					list_size++;
					if (it == last)
						break;
				}
				if (it != last) {
					current_chunk->next = new Chunk<value_type, allocator_type>(N);
					current_chunk = current_chunk->next;
				}
			}
		};

		/// @brief Copy constructor. Constructs the container with the copy of the
		/// contents of other.
		/// @param other another container to be used as source to initialize the
		/// elements of the container with
		ChunkList(const ChunkList& other)  {
			first_chunk = new Chunk<value_type>(N);
			Chunk<value_type>* old_list = other.first_chunk;
			Chunk<value_type>* new_list = first_chunk;

			while (old_list != nullptr) {
				new_list->list = old_list->get_data();
				new_list->chunk_size = old_list->chunk_size;
				if (old_list->next != nullptr) {
					new_list->next = new Chunk<value_type>(N);
					Chunk<value_type>* tmp = new_list;
					new_list = new_list->next;
					new_list->prev = tmp;
				}
				old_list = old_list->next;
			}
			list_size = other.list_size;
		};

		/// @brief Constructs the container with the copy of the contents of other,
		/// using alloc as the allocator.
		/// @param other another container to be used as source to initialize the
		/// elements of the container with
		/// @param alloc allocator to use for all memory allocations of this container
		ChunkList(const ChunkList& other, const Allocator& alloc) {
			this = ChunkList(other);
			Chunk<value_type, allocator_type>* current_chunk = this->first_chunk;
			while (current_chunk != nullptr) {
				current_chunk->allocator = alloc;
				current_chunk = current_chunk->next;
			}
		};

		/**
		 * @brief Move constructor.
		 *
		 * Constructs the container with the contents of other using move semantics.
		 * Allocator is obtained by move-construction from the allocator belonging to
		 * other.
		 *
		 * @param other another container to be used as source to initialize the
		 * elements of the container with
		 */
		ChunkList(ChunkList&& other) {
			first_chunk = std::move(other.first_chunk);
			list_size = std::move(other.list_size);
			other.clear();
		};

		/**
		 * @brief Allocator-extended move constructor.
		 * Using alloc as the allocator for the new container, moving the contents
		 * from other; if alloc != other.get_allocator(), this results in an
		 * element-wise move.
		 *
		 * @param other another container to be used as source to initialize the
		 * elements of the container with
		 * @param alloc allocator to use for all memory allocations of this container
		 */
		ChunkList(ChunkList&& other, const Allocator& alloc) {
			this = ChunkList(other);
			Chunk<value_type, allocator_type>* current_chunk = first_chunk;
			while (current_chunk != nullptr) {
				current_chunk->allocator = alloc;
				current_chunk = current_chunk->next;
			}
		};

		/// @brief Constructs the container with the contents of the initializer list
		/// init.
		/// @param init initializer list to initialize the elements of the container
		/// with
		/// @param alloc allocator to use for all memory allocations of this container
		ChunkList(std::initializer_list<T> init, const Allocator& alloc = Allocator())
			: first_chunk(new Chunk<value_type, allocator_type>(N))
		{
			first_chunk->allocator = alloc;
			auto it = init.begin();
			Chunk<value_type, allocator_type>* current_chunk = first_chunk;
			int count = 0;
			while (it != init.end()) {
				current_chunk->list[count] = *it;
				++count;
				++it;
				if (count == N) {
					current_chunk->next = new Chunk<value_type, allocator_type>(N);
					current_chunk = current_chunk->next;
					count = 0;
				}
			}
			list_size = init.size();
		}

		/// @brief Destructs the ChunkList.
		~ChunkList() {
			clear();
		};

		/// @brief Copy assignment operator. Replaces the contents with a copy of the
		/// contents of other.
		/// @param other another container to use as data source
		/// @return *this
		ChunkList& operator=(const ChunkList& other) {
			(this) = new ChunkList(other);
			return (*this);
		};

		/**
		 * Move assignment operator.
		 *
		 * Replaces the contents with those of other using move semantics
		 * (i.e. the data in other is moved from other into this container).
		 * other is in a valid but unspecified state afterwards.
		 *
		 * @param other another container to use as data source
		 * @return *this
		 */
		ChunkList& operator=(ChunkList&& other) {
			first_chunk = std::move(other.first_chunk);
			list_size = std::move(other.list_size);
			other.clear();

			return *this;
		};

		/// @brief Replaces the contents with those identified by initializer list
		/// ilist.
		/// @param ilist
		/// @return this
		ChunkList& operator=(std::initializer_list<T> ilist) {
			clear();
			auto it = ilist.begin();
			Chunk* current_chunk = first_chunk;
			int count = 0;
			for (const auto& elem : ilist) {
				current_chunk->list[count] = elem;
				++count;
				if (count == N) {
					if (current_chunk->next == nullptr) {
						current_chunk->next = new Chunk<value_type, allocator_type>(N);
						current_chunk->next->prev = current_chunk;
					}
					current_chunk = current_chunk->next;
					count = 0;
				}
			}
			list_size = ilist.size();
			return *this;
		}

		/// @brief Replaces the contents with count copies of value
		/// @param count
		/// @param value
		void assign(size_type count, const T& value) {
			if (count < 0)
				throw std::out_of_range("Count argument must be non-negative");
			clear();
			for (int i = 0; i < count; i++)
				push_back(value);
			list_size = count;
		};

		/// @brief Replaces the contents with copies of those in the range [first,
		/// last).
		/// @tparam InputIt
		/// @param first
		/// @param last
		template <class InputIt>
		void assignIt(InputIt first, InputIt last) {
			clear();
			for (InputIt it = first; it != last; ++it) {
				push_back(*it);
			}
		}

		/// @brief Replaces the contents with the elements from the initializer list
		/// ilis
		/// @param ilist
		void assign(std::initializer_list<T> ilist) {
			clear();
			for (const auto& value : ilist) {
				push_back(value);
			}
		}

		/// @brief Returns the allocator associated with the container.
		/// @return The associated allocator.
		allocator_type get_allocator() const noexcept {
			return first_chunk->allocator;
		};

		Chunk<value_type, allocator_type>* last_chunk() {
			Chunk<value_type, allocator_type>* current_chunk = first_chunk;
			while (current_chunk->next != nullptr)
				current_chunk = current_chunk->next;
			return current_chunk;
		}

		/// ELEMENT ACCESS

		/// @brief Returns a reference to the element at specified location pos, with
		/// bounds checking. If pos is not within the range of the container, an
		/// exception of type std::out_of_range is thrown.
		/// @param pos position of the element to return
		/// @return Reference to the requested element.
		/// @throw std::out_of_range
		reference at(size_type pos) {
			if (pos >= max_size() || pos < 0) {
				throw std::out_of_range("Out of range");
			}
			int chunk_index = pos / chunk_size;
			int elemnt_index = pos % chunk_size;

			Chunk<value_type, allocator_type>* curr_chunk = first_chunk;
			while (chunk_index > 0) {
				curr_chunk = curr_chunk->next;
				chunk_index--;
			}
			return curr_chunk->list[elemnt_index];
		};

		/// @brief Returns a const reference to the element at specified location pos,
		/// with bounds checking. If pos is not within the range of the container, an
		/// exception of type std::out_of_range is thrown.
		/// @param pos position of the element to return
		/// @return Const Reference to the requested element.
		/// @throw std::out_of_range
		const_reference at(size_type pos) const {
			if (pos >= max_size() || pos < 0) {
				throw std::out_of_range("Out of range");
			}
			int chunk_index = pos / N;
			int elemnt_index = pos % N;

			Chunk<value_type, allocator_type>* curr_chunk = first_chunk;
			while (chunk_index > 0) {
				curr_chunk = curr_chunk->next;
				chunk_index--;
			}
			return curr_chunk->list[elemnt_index];
		};

		/// @brief Returns a reference to the element at specified location pos. No
		/// bounds checking is performed.
		/// @param pos position of the element to return
		/// @return Reference to the requested element.
		reference operator[](difference_type pos) {
			int chunk_index = pos / N;
			int elemnt_index = pos % N;

			Chunk<value_type, allocator_type>* curr_chunk = first_chunk;
			while (chunk_index > 0) {
				curr_chunk = curr_chunk->next;
				chunk_index--;
			}
			return curr_chunk->list[elemnt_index];
		};

		/// @brief Returns a const reference to the element at specified location pos.
		/// No bounds checking is performed.
		/// @param pos position of the element to return
		/// @return Const Reference to the requested element.
		const_reference operator[](difference_type pos) const {
			int chunk_index = pos / N;
			int elemnt_index = pos % N;

			Chunk<value_type, allocator_type>* curr_chunk = first_chunk;
			while (chunk_index > 0) {
				curr_chunk = curr_chunk->next;
				chunk_index--;
			}
			return curr_chunk->list[elemnt_index];
		};

		/// @brief Returns a reference to the first element in the container.
		/// Calling front on an empty container is undefined.
		/// @return Reference to the first element
		reference front() {
			if (list_size == 0)
				throw std::logic_error("Empty");

			return first_chunk->list[0];
		};

		/// @brief Returns a const reference to the first element in the container.
		/// Calling front on an empty container is undefined.
		/// @return Const reference to the first element
		const_reference front() const {
			if (list_size == 0)
				throw std::logic_error("Empty");

			return first_chunk->list[0];
		};

		/// @brief Returns a reference to the last element in the container.
		/// Calling back on an empty container causes undefined behavior.
		/// @return Reference to the last element.
		reference back() {
			if (list_size == 0)
				throw std::logic_error("Empty");

			Chunk<value_type, allocator_type>* curr_chunk = last_chunk();

			return curr_chunk->list[curr_chunk->num_of_elements - 1];
		};

		/// @brief Returns a const reference to the last element in the container.
		/// Calling back on an empty container causes undefined behavior.
		/// @return Const Reference to the last element.
		const_reference back() const {
			if(list_size == 0)
				throw std::logic_error("Empty");

			Chunk<value_type, allocator_type>* curr_chunk = last_chunk();

			return curr_chunk->list[curr_chunk->num_of_elements - 1];
		};

		/// ITERATORS

		/// @brief Returns an iterator to the first element of the ChunkList.
		/// If the ChunkList is empty, the returned iterator will be equal to end().
		/// @return Iterator to the first element.
		iterator begin() noexcept {
			return ChunkList_iterator<T>(this, 0, &at(0));
		};

		/// @brief Returns an iterator to the first element of the ChunkList.
		/// If the ChunkList is empty, the returned iterator will be equal to end().
		/// @return Iterator to the first element.
		const_iterator begin() const noexcept {
			return ChunkList_const_iterator<T>(this, 0, &at(0));
		};

		/// @brief Same to begin()
		const_iterator cbegin() const noexcept { return begin(); };

		/// @brief Returns an iterator to the element following the last element of
		/// the ChunkList. This element acts as a placeholder; attempting to access it
		/// results in undefined behavior.
		/// @return Iterator to the element following the last element.
		iterator end() noexcept {
			return ChunkList_iterator<T>();
		};

		/// @brief Returns an constant iterator to the element following the last
		/// element of the ChunkList. This element acts as a placeholder; attempting to
		/// access it results in undefined behavior.
		/// @return Constant Iterator to the element following the last element.
		const_iterator end() const noexcept {
			return ChunkList_const_iterator<T>();
		};

		/// @brief Same to end()
		const_iterator cend() const noexcept { return end(); };

		/// CAPACITY

		/// @brief Checks if the container has no elements
		/// @return true if the container is empty, false otherwise
		bool empty() const noexcept { return list_size == 0; };

		/// @brief Returns the number of elements in the container
		/// @return The number of elements in the container.
		size_type size() const noexcept { return list_size; };

		/// @brief Returns the maximum number of elements the container is able to
		/// hold due to system or library implementation limitations
		/// @return Maximum number of elements.
		size_type max_size() const noexcept {
			int r = list_size % chunk_size;
			return (r == 0 ? list_size : list_size + N - r);
		};

		/// @brief Requests the removal of unused capacity.
		/// It is a non-binding request to reduce the memory usage without changing
		/// the size of the sequence. All iterators and references are invalidated.
		/// Past-the-end iterator is also invalidated.
		void shrink_to_fit() {
			Chunk<value_type, allocator_type>* curr_chunk = last_chunk();

			// We are at the last chunk, release the extra memory
			Chunk<value_type, allocator_type>* new_chunk = new Chunk<value_type, allocator_type>(N);
			for (size_type i = 0; i < curr_chunk->chunk_size; ++i) {
				new_chunk->list[i] = std::move<value_type>(curr_chunk->list[i]);
			}
			new_chunk->num_of_elements = curr_chunk->num_of_elements;

			delete[] curr_chunk->list;
			curr_chunk->list = new_chunk->list;
			curr_chunk->next = new_chunk;
		}

		/// MODIFIERS

		/// @brief Erases all elements from the container.
		/// nvalidates any references, pointers, or iterators referring to contained
		/// elements. Any past-the-end iterators are also invalidated.
		void clear() noexcept {
			Chunk<value_type, allocator_type>* cur = first_chunk;
			while (cur != nullptr) {
				Chunk<value_type, allocator_type>* tmp = cur;
				cur = cur->next;
				delete tmp;
			}
			list_size = 0;
			first_chunk = nullptr;
		};

		/// @brief Inserts value before pos.
		/// @param pos iterator before which the content will be inserted.
		/// @param value element value to insert
		/// @return Iterator pointing to the inserted value.
		iterator insert(const_iterator pos, const T& value) {
			if (pos == cend()) {
				push_back(value);
				return end();
			}
			int index = pos.get_index();
			int i = 0;
			ChunkList_iterator<T> tmpPos = pos.constIteratorToIterator();
			Chunk<value_type, allocator_type>* curr_chunk = last_chunk();
			if (max_size() > list_size) {
				ChunkList_iterator<T> it = ChunkList_iterator<T>(
					this,
					list_size - 1,
					&curr_chunk->list[curr_chunk->chunk_size - 1]
				);
				for (; it >= tmpPos; it--, i++)
					at(list_size - i) = at(list_size - i - 1);
				list_size++;
			}
			else {
				curr_chunk->next = new Chunk<value_type, allocator_type>(N);
				ChunkList_iterator<T> it = ChunkList_iterator<T>(
					this,
					list_size - 1,
					&curr_chunk->list[curr_chunk->chunk_size - 1]
				);
				Chunk<value_type, allocator_type>* tmp = curr_chunk;
				curr_chunk = curr_chunk->next;
				curr_chunk->prev = tmp;
				list_size++;
				for (; it >= tmpPos; it--, i++)
					at(list_size - 1 - i) = at(list_size - i - 2);
			}
			at(index) = value;
			curr_chunk->num_of_elements++;
			return ChunkList_iterator<T>(this, index, &at(index));
		};

		/// @brief Inserts value before pos.
		/// @param pos iterator before which the content will be inserted.
		/// @param value element value to insert
		/// @return Iterator pointing to the inserted value.
		iterator insert(const_iterator pos, T&& value) {
			if (pos == cend()) {
				push_back(value);
				return end();
			}
			int index = pos.get_index();
			int i = 0;
			ChunkList_iterator<T> tmpPos = pos.constIteratorToIterator();
			Chunk<value_type, allocator_type>* curr_chunk = last_chunk();
			if (max_size() > list_size) {
				ChunkList_iterator<T> it = ChunkList_iterator<T>(
					this,
					list_size - 1,
					&curr_chunk->list[curr_chunk->chunk_size - 1]
				);
				for (; it >= tmpPos; it--, i++)
					at(list_size - i) = at(list_size - i - 1);
				list_size++;
			}
			else {
				curr_chunk->next = new Chunk<value_type, allocator_type>(N);
				ChunkList_iterator<T> it = ChunkList_iterator<T>(
					this,
					list_size - 1,
					&curr_chunk->list[curr_chunk->chunk_size - 1]
				);
				Chunk<value_type, allocator_type>* tmp = curr_chunk;
				curr_chunk = curr_chunk->next;
				curr_chunk->prev = tmp;
				list_size++;
				for (; it >= tmpPos; it--, i++)
					at(list_size - 1 - i) = at(list_size - i - 2);
			}
			at(index) = std::move(value);
			curr_chunk->num_of_elements++;
			return ChunkList_iterator<T>(this, index, &at(index));
		};

		private:
		Chunk<value_type, allocator_type>* get_chunk_at_index(size_type index) const {
			int chunk_index = index / N;
			Chunk<value_type, allocator_type>* curr_chunk = first_chunk;
			while (chunk_index > 0) {
				curr_chunk = curr_chunk->next;
				chunk_index--;
			}
			return curr_chunk;
		}

		size_type get_start_index_of_chunk(Chunk<value_type, allocator_type>* chunk) const {
			size_type index = 0;
			Chunk<value_type, allocator_type>* curr_chunk = first_chunk;
			while (curr_chunk != chunk) {
				index += N;
				curr_chunk = curr_chunk->next;
			}
			return index;
		}

		public:
		/// @brief Inserts count copies of the value before pos.
		/// @param pos iterator before which the content will be inserted.
		/// @param count number of elements to insert
		/// @param value element value to insert
		/// @return Iterator pointing to the first element inserted, or pos if count
		/// == 0.
		iterator insert(const_iterator pos, size_type count, const T& value) {
			if (count == 0) {
				return ChunkList_iterator<T>(this, pos.get_index(), &at(pos.get_index()));
			}
			if (pos == cend()) {
				for (size_type i = 0; i < count; ++i) {
					push_back(value);
				}
				return ChunkList_iterator<T>(this, list_size - 1, &at(list_size - 1));
			}

			size_type index = pos.get_index();
			Chunk<value_type, allocator_type>* curr_chunk = get_chunk_at_index(index);
			size_type offset = index - get_start_index_of_chunk(curr_chunk);

			while (count > 0) {
				if (curr_chunk->num_of_elements < N - offset) {
					size_type num_to_copy = std::min(count, N - offset);
					std::copy_backward(curr_chunk->list + offset, curr_chunk->list + curr_chunk->num_of_elements,
						curr_chunk->list + curr_chunk->num_of_elements + num_to_copy);
					std::fill(curr_chunk->list + offset, curr_chunk->list + offset + num_to_copy, value);
					count -= num_to_copy;
					offset = 0;
					curr_chunk->num_of_elements = N;
				}
				else {
					Chunk<value_type, allocator_type>* new_chunk = new Chunk<value_type, allocator_type>();
					new_chunk->prev = curr_chunk;
					new_chunk->next = curr_chunk->next;
					if (curr_chunk->next != nullptr) {
						curr_chunk->next->prev = new_chunk;
					}
					curr_chunk->next = new_chunk;
					curr_chunk = new_chunk;
				}
			}

			list_size += count;
			return ChunkList_iterator<T>(this, index, &at(index));
		}

		Chunk<value_type, allocator_type>* insert_chunk_after(Chunk<value_type, allocator_type>* chunk) {
			Chunk<value_type, allocator_type>* new_chunk = new Chunk<value_type, allocator_type>();
			new_chunk->next = chunk->next;
			new_chunk->prev = chunk;
			if (chunk->next != nullptr) {
				chunk->next->prev = new_chunk;
			}
			chunk->next = new_chunk;
			return new_chunk;
		}

		/// @brief Inserts elements from range [first, last) before pos.
		/// @tparam InputIt Input Iterator
		/// @param pos iterator before which the content will be inserted.
		/// @param first,last the range of elements to insert, can't be iterators into
		/// container for which insert is called
		/// @return Iterator pointing to the first element inserted, or pos if first
		/// == last.
		template <class InputIt>
		iterator insert(const_iterator pos, InputIt first, InputIt last) {
			size_type index = pos.get_index();
			Chunk<value_type, allocator_type>* curr_chunk = get_chunk_at_index(index);
			size_type offset = index - get_start_index_of_chunk(curr_chunk);

			while (first != last) {
				if (offset == N) {
					curr_chunk = insert_chunk_after(curr_chunk);
					offset = 0;
				}
				if (curr_chunk->num_of_elements < N - offset) {
					size_type num_to_copy = std::min(static_cast<size_type>(std::distance(first, last)), N - offset);
					std::copy(first, first + num_to_copy, curr_chunk->list + offset);
					std::advance(first, num_to_copy);
					offset += num_to_copy;
					curr_chunk->num_of_elements += num_to_copy;
				}
			}

			list_size += std::distance(first, last);
			return ChunkList_iterator<T>(this, index, &at(index));
		}

		/// @brief Inserts elements from initializer list before pos.
		/// @param pos iterator before which the content will be inserted.
		/// @param ilist initializer list to insert the values from
		/// @return Iterator pointing to the first element inserted, or pos if ilist
		/// is empty.
		iterator insert(const_iterator pos, std::initializer_list<T> ilist) {
			size_type index = pos.get_index();
			Chunk<value_type, allocator_type>* curr_chunk = get_chunk_at_index(index);
			size_type offset = index - get_start_index_of_chunk(curr_chunk);

			for (const auto& value : ilist) {
				if (offset == N) {
					curr_chunk = insert_chunk_after(curr_chunk);
					offset = 0;
				}
				if (curr_chunk->num_of_elements < N - offset) {
					curr_chunk->list[offset] = value;
					++offset;
					++curr_chunk->num_of_elements;
				}
			}

			list_size += ilist.size();
			return ChunkList_iterator<T>(this, index, &at(index));
		}

		/// @brief Inserts a new element into the container directly before pos.
		/// @param pos iterator before which the new element will be constructed
		/// @param ...args arguments to forward to the constructor of the element
		/// @return terator pointing to the emplaced element.
		template <class... Args>
		iterator emplace(const_iterator pos, Args&&... args) {
			return insert(pos, T(std::forward<Args>(args)...));
		}

		/// @brief Removes the element at pos.
		/// @param pos iterator to the element to remove
		/// @return Iterator following the last removed element.
		iterator erase(const_iterator pos) {
			auto index = pos.get_index();
			Chunk<value_type, allocator_type>* curr_chunk = last_chunk();

			if (index + 1 == list_size) {
				list_size--;
				curr_chunk->num_of_elements--;
				return end();
			}

			for (int i = index + 1; i < list_size; i++) {
				at(i - 1) = at(i);
			}

			if (list_size == max_size() - N + 1) {
				curr_chunk = curr_chunk->prev;
				delete curr_chunk->next;
				curr_chunk->next = nullptr;
			}
			else {
				curr_chunk->num_of_elements--;
			}

			list_size--;
			return ChunkList_iterator<T>(this, index, &at(index));
		};

		/// @brief Removes the elements in the range [first, last).
		/// @param first,last range of elements to remove
		/// @return Iterator following the last removed element.
		iterator erase(const_iterator first, const_iterator last) {
			auto start_index = first.get_index();
			auto end_index = last.get_index();

			// Сдвигаем элементы влево, удаляя элементы в указанном диапазоне
			size_type shift = end_index - start_index + 1;
			for (size_type i = end_index + 1; i < list_size; ++i) {
				at(i - shift) = at(i);
			}

			// Обновляем переменную общее кол-во элементов
			list_size -= shift;

			// Проверяем и обновляем чанки при необходимости
			Chunk<value_type, allocator_type>* start_chunk = get_chunk_at_index(start_index);
			Chunk<value_type, allocator_type>* end_chunk = get_chunk_at_index(end_index);
			if (end_chunk != start_chunk) {
				start_chunk->next = end_chunk->next;
				if (end_chunk->next != nullptr) {
					end_chunk->next->prev = start_chunk;
				}
				else {
					end_chunk = start_chunk;
				}
			}
			end_chunk->num_of_elements -= shift - (end_index % N) - 1;

			// Возвращаем итератор, указывающий на первый элемент после удаленного диапазона
			return ChunkList_iterator<T>(this, start_index, &at(start_index));
		}

		/// @brief Appends the given element value to the end of the container.
		/// The new element is initialized as a copy of value.
		/// @param value the value of the element to append
		void push_back(const T& value) {
			if (first_chunk == nullptr) {
				first_chunk = new Chunk<value_type, allocator_type>(N);
			}

			Chunk<value_type, allocator_type>* curr_chunk = last_chunk();
			if (curr_chunk->num_of_elements == N) {
				curr_chunk->next = new Chunk<value_type, allocator_type>(N);
				Chunk<value_type, allocator_type>* prev = curr_chunk;
				curr_chunk = curr_chunk->next;
				curr_chunk->prev = prev;
			}
			curr_chunk->list[curr_chunk->num_of_elements++] = value;
			list_size++;
		}

		/// @brief Appends the given element value to the end of the container.
		/// Value is moved into the new element.
		/// @param value the value of the element to append
		void push_back(T&& value) {
			if (first_chunk == nullptr)
				first_chunk = new Chunk<value_type, allocator_type>(N);

			Chunk<value_type, allocator_type>* curr_chunk = last_chunk();
			if (curr_chunk->num_of_elements == N)
			{
				curr_chunk->next = new Chunk<value_type, allocator_type>(N);
				Chunk<value_type, allocator_type>* prev = curr_chunk;
				curr_chunk = curr_chunk->next;
				curr_chunk->prev = prev;
			}
			curr_chunk->list[curr_chunk->num_of_elements++] = std::move(value);
			list_size++;
		};

		/// @brief Appends a new element to the end of the container.
		/// @param ...args arguments to forward to the constructor of the element
		/// @return A reference to the inserted element.
		template <class... Args>
		reference emplace_back(Args&&... args) {
			if (first_chunk == nullptr) {
				first_chunk = new Chunk<value_type, allocator_type>(N);
			}

			Chunk<value_type, allocator_type>* curr_chunk = last_chunk();
			if (curr_chunk->num_of_elements == N) {
				curr_chunk->next = new Chunk<value_type, allocator_type>(N);
				curr_chunk = curr_chunk->next;
			}

			curr_chunk->list[curr_chunk->num_of_elements++] = value_type(std::forward<Args>(args)...);;
			list_size++;
			return curr_chunk->list[curr_chunk->num_of_elements - 1];
		}

		/// @brief Removes the last element of the container.
		void pop_back() {
			if (list_size == 0) {
				return;
			}

			list_size--;
			Chunk<value_type, allocator_type>* curr_chunk = last_chunk();
			curr_chunk->list[curr_chunk->num_of_elements - 1] = (value_type)nullptr;
			curr_chunk->num_of_elements--;

			if (curr_chunk->num_of_elements == 0 && first_chunk != curr_chunk) {
				Chunk<value_type, allocator_type>* prev_chunk = curr_chunk->prev;
				delete curr_chunk;
				prev_chunk->next = nullptr;
			}
		}

		/// @brief Prepends the given element value to the beginning of the container.
		/// @param value the value of the element to prepend
		void push_front(const T& value) {
			insert(cbegin(), value);
		};

		/// @brief Prepends the given element value to the beginning of the container.
		/// @param value moved value of the element to prepend
		void push_front(T&& value) {
			insert(cbegin(), std::move(value));
		};

		/// @brief Inserts a new element to the beginning of the container.
		/// @param ...args arguments to forward to the constructor of the element
		/// @return A reference to the inserted element.
		template <class... Args>
		reference emplace_front(Args&&... args) {
			auto it = insert(cbegin(), value_type(std::forward<Args>(args)...));
			return *it;
		};

		/// @brief Removes the first element of the container.
		void pop_front() {
			erase(cbegin());
		};

		/// @brief Resizes the container to contain count elements.
		/// If the current size is greater than count, the container is reduced to its
		/// first count elements. If the current size is less than count, additional
		/// default-inserted elements are appended
		/// @param count new size of the container
		void resize(size_type count) {
			if (count < 0)
				throw std::invalid_argument("Count can not be negative");

			Chunk<value_type, allocator_type>* curr_chunk = first_chunk;

			while (curr_chunk != nullptr) {
				curr_chunk->resize(count);
				curr_chunk = curr_chunk->next;
			}

			if (count < N) {
				list_size = 0;

				curr_chunk = first_chunk;
				while (curr_chunk != nullptr) {
					list_size += curr_chunk->num_of_elements;
					curr_chunk = curr_chunk->next;
				}
			}

			chunk_size = count;
		};

		/// @brief Resizes the container to contain count elements.
		/// If the current size is greater than count, the container is reduced to its
		/// first count elements. If the current size is less than count, additional
		/// copies of value are appended.
		/// @param count new size of the container
		/// @param value the value to initialize the new elements with
		void resize(size_type count, const value_type& value) {
			if (count < 0)
				throw std::invalid_argument("Count can not be negative");

			Chunk<value_type, allocator_type>* curr_chunk = first_chunk;

			while (curr_chunk != nullptr) {
				curr_chunk->resize(count, value);
				curr_chunk = curr_chunk->next;
			}

			if (count < N) {
				list_size = 0;

				curr_chunk = first_chunk;
				while (curr_chunk != nullptr) {
					list_size += curr_chunk->num_of_elements;
					curr_chunk = curr_chunk->next;
				}
			}

			chunk_size = count;
		};

		/// @brief Exchanges the contents of the container with those of other.
		/// Does not invoke any move, copy, or swap operations on individual elements.
		/// All iterators and references remain valid. The past-the-end iterator is
		/// invalidated.
		/// @param other container to exchange the contents with
		void swap(ChunkList<T, N, Allocator>& other) {
			std::swap(other.first_chunk, first_chunk);
			std::swap(other.list_size, list_size);
		}

		/// @brief Print the content of ChunkList to console.
		void print() {
			int chunk_num = 1;
			Chunk<value_type, allocator_type>* curr_chunk = first_chunk;
			while (true) {
				std::cout << "Chunk num: " << chunk_num << std::endl;
				for (value_type* el = curr_chunk->begin(); el != curr_chunk->end(); el++)
					std::cout << *el << "\t";
				std::cout << std::endl;

				chunk_num++;

				if (curr_chunk->next == nullptr) {
					break;
				}
				else {
					curr_chunk = curr_chunk->next;
				}
			}
		}

		/// COMPARISIONS

		/// @brief Checks if the contents of lhs and rhs are equal
		/// @param lhs,rhs ChunkLists whose contents to compare
		friend bool operator==(const ChunkList<value_type, N, allocator_type>& lhs,
			const ChunkList<value_type, N, allocator_type>& rhs) {
			if (lhs.list_size != rhs.list_size)
				return false;

			for (int i = 0; i < lhs.list_size; i++)
				if (lhs.at(i) != rhs.at(i))
					return false;

			return true;
		};

		/// @brief Checks if the contents of lhs and rhs are not equal
		/// @param lhs,rhs ChunkLists whose contents to compare
		friend bool operator!=(const ChunkList<value_type, N, allocator_type>& lhs,
			const ChunkList<value_type, N, allocator_type>& rhs) {
			return !operator==(lhs, rhs);
		};

		/// @brief Compares the contents of lhs and rhs lexicographically.
		/// @param lhs,rhs ChunkLists whose contents to compare
		friend bool operator>(const ChunkList<value_type, N, allocator_type>& lhs,
			const ChunkList<value_type, N, allocator_type>& rhs) {
			if (lhs.list_size != rhs.list_size) {
				return lhs.list_size > rhs.list_size;
			}
			else {
				for (int i = 0; i < lhs.list_size; i++)
					if (lhs.at(i) <= rhs.at(i))
						return false;

				return true;
			}
		};

		/// @brief Compares the contents of lhs and rhs lexicographically.
		/// @param lhs,rhs ChunkLists whose contents to compare
		friend bool operator<(const ChunkList<value_type, N, allocator_type>& lhs,
			const ChunkList<value_type, N, allocator_type>& rhs) {
			return !operator>(lhs, rhs);
		};

		/// @brief Compares the contents of lhs and rhs lexicographically.
		/// @param lhs,rhs ChunkLists whose contents to compare
		friend bool operator>=(const ChunkList<value_type, N, allocator_type>& lhs,
			const ChunkList<value_type, N, allocator_type>& rhs) {
			if (lhs.list_size < rhs.list_size) {
				return false;
			}
			else {
				for (int i = 0; i < lhs.list_size; i++)
					if (lhs.at(i) < rhs.at(i))
						return false;

				return true;
			}
		};

		/// @brief Compares the contents of lhs and rhs lexicographically.
		/// @param lhs,rhs ChunkLists whose contents to compare
		friend bool operator<=(const ChunkList<value_type, N, allocator_type>& lhs,
			const ChunkList<value_type, N, allocator_type>& rhs) {
			return !operator>=(lhs, rhs);
		};

		// operator <=> will be handy
		std::strong_ordering operator<=>(const ChunkList& other) const {
			if (list_size < other.list_size) {
				return std::strong_ordering::less;
			}
			else if (list_size > other.list_size) {
				return std::strong_ordering::greater;
			}
			else {
				for (int i = 0; i < list_size; i++) {
					const auto& l = at(i);
					const auto& r = other.at(i);
					if (l < r) {
						return std::strong_ordering::less;
					}
					else if (l > r) {
						return std::strong_ordering::greater;
					}
				}
				return std::strong_ordering::equal;
			}
		}
	};

	/// NON-MEMBER FUNCTIONS

	/// @brief  Swaps the contents of lhs and rhs.
	/// @param lhs,rhs containers whose contents to swap
	template <class T, int N, class Alloc>
	void swap(ChunkList<T, N, Alloc>& lhs, ChunkList<T, N, Alloc>& rhs);

	/// @brief Erases all elements that compare equal to value from the container.
	/// @param c container from which to erase
	/// @param value value to be removed
	/// @return The number of erased elements.
	template <class T, int N, class Alloc, class U>
	typename ChunkList<T, N, Alloc>::size_type erase(ChunkList<T, N, Alloc>& c, const U& value);

	/// @brief Erases all elements that compare equal to value from the container.
	/// @param c container from which to erase
	/// @param pred unary predicate which returns ​true if the element should be
	/// erased.
	/// @return The number of erased elements.
	template <class T, int N, class Alloc, class Pred>
	typename ChunkList<T, N, Alloc>::size_type erase_if(ChunkList<T, N, Alloc>& c, Pred pred);
}  // namespace fefu_laboratory_two

