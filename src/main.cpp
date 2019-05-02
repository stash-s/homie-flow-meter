#include <Homie.h>

HomieNode flowNode("flow", "Flow", "flow");

bool broadcastHandler(const String &level, const String &value)
{
    //Homie.getLogger() << "Received broadcast level " << level << ": " << value << endl;
    return true;
}

const int SENSOR_PIN = D5;
unsigned long flow_count = 0;
unsigned long flow_ratio = 1000;

//1000;

ICACHE_RAM_ATTR void
flow_meter_isr()
{
    ++flow_count;
}

const unsigned long FLOW_INTERVAL = 1000UL;
const float beats_per_litre = 373.3962;

unsigned long last_update_time = 0;
unsigned long last_state_seen = 0;

unsigned int seconds = 0;

inline String to_string(float value)
{
    return String(value, 5);
}

void loopHandler()
{
    unsigned long time_now = millis();
    unsigned long time_elapsed = time_now - last_update_time;

    if (time_elapsed >= FLOW_INTERVAL || last_update_time == 0)
    {
        unsigned long incremental_beats = flow_count - last_state_seen;

        float cumulative_flow = (float)flow_count / beats_per_litre;

        float flow_per_minute = 60000.0 * ((float)incremental_beats / ((float)time_elapsed * beats_per_litre));
        float incremental_flow = (float)incremental_beats / beats_per_litre;

        float beats_per_sec = 1000 * (float)incremental_beats / ((float)time_elapsed);

        last_state_seen = flow_count;
        last_update_time = time_now;

        flowNode.setProperty("flow").send(to_string(flow_per_minute));
        flowNode.setProperty("beats").send(String(beats_per_sec));

        flowNode.setProperty("cumulative-flow").send(to_string(cumulative_flow));
        flowNode.setProperty("cumulative-beats").send(String(flow_count));

        flowNode.setProperty("incremental-flow").send(to_string(incremental_flow));
        flowNode.setProperty("incremental-beats").send(String(incremental_beats));
    }
}

void setup()
{
    Serial.begin(9600);
    Serial << endl
           << endl;
    Homie_setFirmware("flow-meter", "1.0.0");
    Homie.setBroadcastHandler(broadcastHandler);
    Homie.setLoopFunction(loopHandler);

    flowNode.advertise("flow").setName("Litres per minute").setDatatype("float").setUnit("l/min");
    flowNode.advertise("beats").setName("Beats per second").setDatatype("integer").setUnit("bps");

    flowNode.advertise("cumulative-flow").setName("Cumulative flow").setDatatype("float").setUnit("litres");
    flowNode.advertise("cumulative-beats").setName("Cumulative beats").setDatatype("integer").setUnit("beats");

    flowNode.advertise("incremental-flow").setName("Incremental flow").setDatatype("float").setUnit("litres");
    flowNode.advertise("incremental-beats").setName("Incremental beats").setDatatype("integer").setUnit("beats");

    Homie.setup();

    //    ota_setup();

    attachInterrupt(SENSOR_PIN, flow_meter_isr, RISING);
}

void loop()
{
    Homie.loop();

    //    ota_handle();
}