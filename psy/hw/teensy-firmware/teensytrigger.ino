#include <_Teensy.h>
#include <stdint.h>

#define PIN_MASK ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 3))

class State {
  public:
    State() : m_connected(false) {}

    void set_connected(bool connected) noexcept { m_connected = connected; }

    bool connected() const noexcept { return m_connected; }

  private:
    bool m_connected;
};

State g_state;

class Msg {

  public:
    typedef enum msg_type {
        NOT_INITIALIZED = -1,
        CONNECT         = 1,
        ACK,
        ERROR,
        PIN_OUT,
        CLOSE,
    } MsgType;

    static const uint8_t header_size = 2;

    static Msg create_ack()
    {
        Msg ret = create_simple_msg(ACK);
        return ret;
    }

    static Msg create_simple_msg(MsgType type)
    {
        Msg ret;
        ret.buffer[TYPE_INDEX] = type;
        ret.buffer[SIZE_INDEX] = header_size;
        return ret;
    }

    static Msg create_error(const String& msg)
    {
        Msg ret;
        ret.buffer[TYPE_INDEX] = ERROR;
        ret.buffer[SIZE_INDEX]
            = min(static_cast<int>(msg.length() + header_size), 256);

        const char       *read_ptr  = msg.c_str();
        char             *write_ptr = &ret.buffer[header_size];
        const char *const end_ptr   = write_ptr + ret.payload_size();

        while (write_ptr < end_ptr)
            *write_ptr++ = *read_ptr++;

        return ret;
    }

    static Msg read_from_serial(void)
    {
        Msg ret;

        uint8_t n_to_read = header_size;
        uint8_t n_read    = 0;

        while (n_to_read - n_read) {
            n_read += Serial.readBytes(&ret.buffer[n_read], Msg::header_size);
        }

        n_to_read = ret.msg_size();
        while (n_to_read - n_read) {
            n_read
                += Serial.readBytes(&ret.buffer[n_read],
                                    min(256 - header_size, n_to_read - n_read));
        }

        ret.zero_terminate();

        return ret;
    }

    void send(void) const noexcept
    {
        size_t n_to_write = this->msg_size();
        size_t n_written  = 0;

        while (n_written < n_to_write) {
            n_written += Serial.write(&this->buffer[n_written],
                                      n_to_write - n_written);
        }
    }

    uint8_t msg_size() const noexcept { return this->buffer[0]; }

    uint8_t payload_size() const noexcept { return msg_size() - header_size; }

    Msg::MsgType type() const noexcept
    {
        return static_cast<MsgType>(buffer[TYPE_INDEX]);
    }

    const char *payload() const noexcept { return &buffer[PAYLOAD_INDEX]; }

    void zero_terminate() noexcept
    {
        buffer[min(sizeof(buffer) - 1, msg_size())] = 0;
    }

  private:
    static const int PAYLOAD_INDEX = 2;
    static const int TYPE_INDEX    = 1;
    static const int SIZE_INDEX    = 0;

    /**
     * The buffer is is {size, type, [p,a,y,l,o,a,d]}
     * the size is the number of bytes that need to be send
     * or recieved in order to compute the total message.
     * The type tells what message it is.
     * the payload can be up to 254 bytes, however, if
     * the payload is a string the a terminating 0 will
     * be set at the last byte, so a string might only
     * have a length of 253 bytes.
     */
    char buffer[256];
};

void
setup()
{
    // put your setup code here, to run once:
    Serial.begin(38400);

    // Set all pins in output mode
    for (int i = 0; i < 32; i++)
        pinMode(i, OUTPUT);
    GPIOD_PDOR &= PIN_MASK;
}

void
handle_connect(const Msg& msg)
{
    String connection_msg = String(msg.payload());
    Msg    m;
    if (connection_msg == "client connect") {
        m = Msg::create_ack();
        g_state.set_connected(true);
        GPIOD_PDOR = 0x0; // clear output.
    }
    else {
        m = Msg::create_error("Unexpected connect message payload: '"
                              + connection_msg + "'");
        g_state.set_connected(false);
    }
    m.send();
}

void
handle_close()
{
    Msg m = Msg::create_ack();
    m.send();
    GPIOD_PDOR = 0; // clear all pins
    g_state.set_connected(false);
}

void
handle_pin_out(const Msg& msg)
{
    if (g_state.connected()) {
        uint32_t pins = msg.payload()[0];
        GPIOD_PDOR    = pins;
    }
}

void
handle_message(const Msg& msg) noexcept
{
    switch (msg.type()) {
    case Msg::CONNECT:
        handle_connect(msg);
        break;
    case Msg::PIN_OUT:
        handle_pin_out(msg);
        break;
    case Msg::CLOSE:
        handle_close();
        break;
    default:
    {
        Msg msg = Msg::create_error(String() + "Unexpected message type"
                                    + String(msg.type()));
    }
    }
}

void
loop(void)
{
    // put your main code here, to run repeatedly:

    // GPIOD is GPIO_register for port D
    // PCOR is Port Clear Output Register
    // PDOR is Port Data Output Register
    // GPIOD_PCOR = PIN_MASK;

    if (Serial.available()) {

        Msg msg = Msg::read_from_serial();

        handle_message(msg);
    }
}
