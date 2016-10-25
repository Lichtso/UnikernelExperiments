#include "../Clock.hpp"

struct Mac {
    struct Address {
        Natural8 bytes[6];

        Address& operator=(const Address& other) {
            memcpy(this, &other, sizeof(other));
            return *this;
        }

        bool operator==(const Address& other) {
            return memcmp(this, &other, sizeof(other)) == 0;
        }

        bool operator!=(const Address& other) {
            return memcmp(this, &other, sizeof(other)) != 0;
        }
    };

    struct Frame {
        Address destinationAddress, sourceAddress;
        Natural16 type;
        Natural8 payload[0];

        void correctEndian() {
            swapEndian(type);
        }
    };
    static_assert(sizeof(Frame) == 14);

    struct Interface;
};
