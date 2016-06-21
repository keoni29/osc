#ifndef SMF_H_
#define SMF_H_

#include <stdint.h>

#define SMF_MIDI_CHANNELS 16
#define SMF_MAX_TRACKS 20


/** Error codes for SMF_Load() */
#define SMF_SUCCESS 0			/**< No error */
#define SMF_ERROR_FILE -2		/**< Could not open file */
#define SMF_ERROR_INVALID -3	/**< File is not a valid Standard Midi File. */
#define SMF_ERROR_COMPLEX -4	/**< File is not supported */

/** Return values for SMF_PollEvent() */
#define SMF_EVENT_READY 1
#define SMF_NO_EVENTS 0
#define SMF_STOPPED -1

/** Bits [6..4] of the midi status byte. */
enum SMF_status
{
	SMF_NoteOff,
	SMF_NoteOn,
	SMF_KeyPressure,
	SMF_ControlChange,
	SMF_ChannelPressure,
	SMF_PitchBend,
	SMF_Unhandled
};

struct SMF_header
{
	char str[4];
	uint32_t length;
	uint16_t format;
	uint16_t n;
	uint16_t division;
};

struct SMF_event
{
	uint8_t channel;			/**< MIDI channel 1-16 */
	enum SMF_status status;		/**< Bits [6..4] of the midi status byte. */
	uint8_t p1, p2;				/**< Event parameters */
};

struct SMF_track
{
	char str[4];
	uint32_t head;
	uint32_t length;
	uint8_t *data;
	uint32_t dt;
	uint8_t status;
};

int SMF_Load(char* filename);
int SMF_PollEvent(struct SMF_event *e, uint32_t *dt);

#endif /* SMF_H_ */
