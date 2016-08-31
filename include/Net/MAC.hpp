#include "../Hardware/AllwinnerA64.hpp"

struct MAC {
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

    void initialize();
    void poll();
    Frame* prepareTransmit(Natural16 totalLength);
    void transmit(Frame* frame);

    void setMACAddress(const Address& src);
    void getMACAddress(Address& dst);

    void transmited(Natural32 errors, Natural32 length, Frame* frame);
    void received(Natural32 errors, Natural32 length, Frame* frame);
};
