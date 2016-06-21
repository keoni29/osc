#include <stdio.h>
#include <stdlib.h>
#include "smf.h"
#include "play.h"

static void ReverseInt16(uint16_t *d); /**< Helper function for reversing endianess */
static void ReverseInt32(uint32_t *d); /**< Helper function for reversing endianess */
static uint32_t VData(struct SMF_track *track);	/**< Parse variable length data field */
static uint8_t Data(struct SMF_track *track);	/**< Parse single byte data field */
static uint8_t Peek(const struct SMF_track *track);	/**< Peek single byte data field without incrementing pointer */
static void SkipData(struct SMF_track *track, uint32_t skip);	/**< Skip over several fields */
static int VerifyTrack(struct SMF_track* track);
static int _strcmpl(char *str1, char *str2, uint32_t len);	/**< Compare two strings */

static uint32_t usPerBeat = 500000;
static uint32_t ticksPerBeat;
static uint32_t tick;
static uint16_t trackCount = 0;
static struct SMF_track track[SMF_MAX_TRACKS];


/* Poll MIDI event.
 * Returns 1 on succes, SFM_event contains event and dt contains interval in ms.
 * Returns SMF_STOPPED on playback end.
 */
int SMF_PollEvent(struct SMF_event *e, uint32_t *dt)
{
	int result = SMF_NO_EVENTS;
	//int i;
	int a = 0; /* Todo implement for multiple tracks */
	uint32_t length;
	uint8_t type;

	uint8_t status = track[a].status;

	if (track->head >= track->length)
	{
		return SMF_STOPPED;
	}

	if (!(Peek(&track[a]) & 0x80))
	{
		/* Running byte! Do not change the state. */
	}
	else
	{
		status = Data(&track[a]);
		track[a].status = status;
	}

	if (status == 0xF0 || status == 0xF7)
	{
		/* System exclusive event */
		while (Data(&track[a]) != 0xF7)
		{
			;
		}
	}
	else if (status == 0xFF)
	{
		/* Meta event */
		type = Data(&track[a]);
		length = VData(&track[a]);

		if (type == 0x51 && length == 3)
		{
			usPerBeat = (Data(&track[a]) << 16) | (Data(&track[a]) << 8) | (Data(&track[a]) << 0);
			tick = usPerBeat / ticksPerBeat;
		}
		else
		{
			/* Unknown message */
			SkipData(&track[a], length);
		}
	}
	else
	{
		e->channel = (status & 0x0F) + 1;
		e->status = (status & 0x70) >> 4;
		/* MIDI event */
		switch (status & 0xF0)
		{
		case 0x80:
		case 0x90:
		case 0xA0:
		case 0xB0:
		case 0xE0:
			/* 2 parameters */
			e->p1 = Data(&track[a]);
			e->p2 = Data(&track[a]);
			result = SMF_EVENT_READY;
			break;
		case 0xC0:
		case 0xD0:
			/* 1 parameter */
			e->p1 = Data(&track[a]);
			result = SMF_EVENT_READY;
			break;
		case 0xF0:
		default:
			/* Todo: handle these unhandled events */
			break;
		}
	}

	/* Calculate delta time in us */
	track[a].dt = (VData(&track[a]) * tick) / 1000;

	/* Todo determine smallest dt */
	/* Whose first up? (Smallest delta time.) */
	/*uint32_t min = track[0];
	for (i = 0; i < trackCount; i++)
	{
		if (track[i] < min)
		{
			min = track[i];
		}
	}*/

	/* Calculate delta time in ms */
	//*dt = min / 1000;
	*dt = track[a].dt;

	return result;
}

/** Load standard midi file
 * 	Returns negative error code on failure, 0 on success.
 * 	Remember to call SMF_Free() before loading a new song. */
int SMF_Load(char* filename)
{
	struct SMF_header header;
	FILE *in = NULL;
	int i;

	in = fopen(filename, "r");
	if (in == NULL)
	{
		return SMF_ERROR_FILE;
	}

	/** Todo: handle errors */
	fread((void *)&header.str, sizeof(header.str), 1, in);
	fread((void *)&header.length, sizeof(header.length), 1, in);
	fread((void *)&header.format, sizeof(header.format), 1, in);
	fread((void *)&header.n, sizeof(header.n), 1, in);
	fread((void *)&header.division, sizeof(header.division), 1, in);

	if (!_strcmpl("MThd", header.str, 4))
	{
		return SMF_ERROR_INVALID;
	}

	ReverseInt32(&header.length);
	ReverseInt16(&header.format);
	ReverseInt16(&header.n);
	ReverseInt16(&header.division);

	if (header.n > SMF_MAX_TRACKS)
	{
		return SMF_ERROR_COMPLEX;
	}

	trackCount = header.n;

	ticksPerBeat = header.division;

	/* Load and verify all tracks */
	for (i = 0; i < header.n; i++)
	{
		/* Todo: handle errors */
		fread((void *)&track[i].str, sizeof(track[i].str), 1, in);
		fread((void *)&track[i].length, sizeof(track[i].length), 1, in);

		ReverseInt32(&track[i].length);

		track[i].data = (uint8_t *)malloc(track[i].length);
		if (track[i].data != NULL)
		{
			fread((void *)track[i].data, track[i].length, 1, in);
		}

		if (VerifyTrack(&track[i]) < 0)
		{
			return SMF_ERROR_INVALID;
		}

		/* Rewind head to start */
		track[i].head = 0;
		/* Todo Calculate initial delta time in ms */
		VData(&track[i]);
	}

	return SMF_SUCCESS;
}

void SMF_Free()
{
	int i;
	for (i = 0; i < SMF_MAX_TRACKS; i++)
	{
		free(track[i].data);
	}
}

/** Verify the track
 * 	Returns negative error code on failure, 1 when last event in track was not Meta "End of Track", 0 on success. */
static int VerifyTrack(struct SMF_track* t)
{
	uint8_t status, type;

	if (!_strcmpl("MTrk", t->str, 4))
	{
		return -1;
	}

	while (t->head < t->length)
	{
		uint32_t length;

		/* Skip over delta time field */
		VData(t);

		if (!(Peek(t) & 0x80))
		{
			/* Running byte! Do not change the state. */
		}
		else
		{
			status = Data(t);
		}

		if (status == 0xF0 || status == 0xF7)
		{
			/* System exclusive event */
			while (Data(t) != 0xF7)
			{
				;
			}
		}
		else if (status == 0xFF)
		{
			/* Meta event */
			type = Data(t);
			length = VData(t);
			SkipData(t, length);
		}
		else
		{
			/* MIDI event */
			switch (status & 0xF0)
			{
			case 0x80:
			case 0x90:
			case 0xA0:
			case 0xB0:
			case 0xE0:
				/* 2 parameters */
				SkipData(t, 2);
				break;
			case 0xC0:
			case 0xD0:
				/* 1 parameter */
				SkipData(t, 1);
				break;
			case 0xF0:
			default:
				/* Unhandled event */
				return -1;
				break;
			}
		}
	}

	if (!(status == 0xFF && type == 0x2F))
	{
		/* Last message was not META 0x2F (End of track) */
		return 1;
	}
	return 0;
}



static int _strcmpl(char *str1, char *str2, uint32_t len)
{
	uint32_t i;
	for (i = 0; i < len; i++)
	{
		if (str1[i] != str2[i])
		{
			return 0;
		}
	}
	return 1;
}

static void ReverseInt16(uint16_t *d)
{
	uint8_t help;
	help = *(((uint8_t*)d)+0);
	*(((uint8_t*)d)+0) = *(((uint8_t*)d)+1);
	*(((uint8_t*)d)+1) = help;
}

static void ReverseInt32(uint32_t *d)
{
	uint8_t help1, help2;
	help1 = *(((uint8_t*)d) + 0);
	help2 = *(((uint8_t*)d) + 1);
	*(((uint8_t*)d) + 0) = *(((uint8_t*)d) + 3);
	*(((uint8_t*)d) + 1) = *(((uint8_t*)d) + 2);
	*(((uint8_t*)d) + 2) = help2;
	*(((uint8_t*)d) + 3) = help1;
}

static uint32_t VData(struct SMF_track *track)
{
	uint32_t d = 0;
	int i;

	if (track->head < track->length)
	{
		for (i = 0; i < 4; i++)
		{
			d = (d << 7) + (track->data[track->head] & 0x7F);

			if (!(track->data[track->head] & 0x80))
			{
				++track->head;
				break;
			}
			++track->head;
		}
	}

	return d;
}

static uint8_t Data(struct SMF_track *track)
{
	uint8_t d = track->data[track->head];
	if (track->head < track->length)
	{
		++track->head;
		return d;
	}
	return 0;
}

static uint8_t Peek(const struct SMF_track *track)
{
	if (track->head < track->length)
	{
		return track->data[track->head];
	}
	return 0;
}

static void SkipData(struct SMF_track *track, uint32_t skip)
{
	while (skip && track->head < track->length)
	{
		Data(track);
		--skip;
	}
}
