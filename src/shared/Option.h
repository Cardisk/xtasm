#ifndef OPTION_H
#define OPTION_H

#include <optional>

template <typename T> 
class Option {
    public:
        // Create a new Option with the given value.
        static Option<T> some(T value) {
            return Option<T>(value);
        }
        // Create a new emtpy Option.
        static Option<T> none() {
            return Option<T>({});
        }

        // Check if contains something.
        bool is_some() {
            return this->value.has_value();
        }
        // Check if it doesn't contain anything.
        bool is_none() {
            return !this->is_some();
        }

        // Unsafe unwrap.
        T unwrap() {
            return this->value.value();
        }
        // Safe unwrap.
        T unwrap_or(T alternative) {
            if (this->is_some())
                return this->value.value();
            return alternative;
        }

        // Accepts only Option value (captures not supported).
        bool is_some_and(bool (*predicate) (T)) {
            if (this->is_some())
                return predicate(this->value.value());
            return false;
        }
        // Accepts only Option value (captures not supported).
        bool is_some_and(int (*predicate) (T)) {
            if (this->is_some())
                return (bool) predicate(this->value.value());
            return 0;
        }
        // Accepts Option value and another variable (captures not supported).
        bool is_some_and(bool (*predicate) (T, T), T comparison) {
            if (this->is_some())
                return predicate (this->value.value(), comparison);
            return false;
        }
        // Accepts Option value and another variable (captures not supported).
        bool is_some_and(int (*predicate) (T, T), T comparison) {
            if (this->is_some())
                return (bool) predicate(this->value.value(), comparison);
            return 0;
        }

        // Replace the contained value.
        void insert(T new_value) {
            this->value = new_value;
        }
        // Unsafe removal of the contained value and return it to the caller.
        T take() {
            T obj = this->value.value ();
            this->value = {};
            return obj;
        }
        // Unsafe replacing of the contained value and return a new Option with the old one.
        Option<T> replace(T new_value) {
            Option<T> obj = Option<T>::some(this->value.value());
            this->insert(new_value);
            return obj;
        }

        // Get a reference of the contained value.
        const T &as_ref() {
            return this->value.value();
        }

    private:
        // Explicit c'tor.
        explicit Option(std::optional<T> value) { this->value = value; }

        std::optional<T> value;
};

#endif // OPTION_H
