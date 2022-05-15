


#include <main.h>

messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);



static void serial_start(void)
{
	static SerialConfig ser_cfg = {
	    115200,
	    0,
	    0,
	    0,
	};

	sdStart(&SD3, &ser_cfg); // UART3.
}



int main(void)
{
    halInit();
    chSysInit();
    proximity_start();
    //spi_comm_start();
    messagebus_init(&bus, &bus_lock, &bus_condvar);

    //starts the serial communication
    serial_start();
    //starts the USB communication
    usb_start();
    //inits the motors
    motors_init();
    //inits our threads

    Move_start();
    ObstacleInspector_start();
    init_position_direction();

    mic_start(&processAudioData);


    /* Infinite loop. */
    while (1) { //put to sleep for 500 ms
    	chThdSleepMilliseconds(500);
    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void){
    chSysHalt("Stack smashing detected");
}
