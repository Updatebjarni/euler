#include<alsa/asoundlib.h>
#include<stdio.h>

#define NAME "midi.c"

int main(){
  snd_seq_t *seq;
  int in_port, client, queue;
  snd_seq_addr_t port={.client=128, .port=0};
  snd_seq_port_info_t *pinfo;
  snd_seq_event_t ev;

  printf("open: %d\n", snd_seq_open(&seq, "default", SND_SEQ_OPEN_DUPLEX, 0));
  snd_seq_set_client_name(seq, NAME);
  client=snd_seq_client_id(seq);

  snd_seq_port_info_malloc(&pinfo);
  snd_seq_port_info_set_port(pinfo, 0);
  snd_seq_port_info_set_port_specified(pinfo, 1);
  snd_seq_port_info_set_name(pinfo, NAME);
  snd_seq_port_info_set_capability(pinfo, 0); /* sic */
  snd_seq_port_info_set_type(
    pinfo, SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
  snd_seq_create_port(seq, pinfo);

  queue=snd_seq_alloc_named_queue(seq, NAME);
  printf("connect: %d\n", snd_seq_connect_to(seq, 0, port.client, port.port));

snd_seq_ev_clear(&ev);
ev.queue = queue;
ev.source.port = 0;
ev.flags = SND_SEQ_TIME_STAMP_TICK|SND_SEQ_TIME_MODE_REL;
ev.type = SND_SEQ_EVENT_NOTEON;
ev.time.tick = 0;
ev.dest = port;
ev.data.note.channel = 0;
ev.data.note.note = 60;
ev.data.note.velocity = 127;
snd_seq_event_output(seq, &ev);
snd_seq_drain_output(seq);

getchar();

snd_seq_ev_clear(&ev);
ev.queue = queue;
ev.source.port = 0;
ev.flags = SND_SEQ_TIME_STAMP_TICK|SND_SEQ_TIME_MODE_REL;
ev.type = SND_SEQ_EVENT_NOTEOFF;
ev.time.tick = 0;
ev.dest = port;
ev.data.note.channel = 0;
ev.data.note.note = 60;
ev.data.note.velocity = 0;
snd_seq_event_output(seq, &ev);
snd_seq_drain_output(seq);

snd_seq_ev_set_fixed(&ev);
ev.type = SND_SEQ_EVENT_STOP;
ev.time.tick = 0;
ev.dest.client = SND_SEQ_CLIENT_SYSTEM;
ev.dest.port = SND_SEQ_PORT_SYSTEM_TIMER;
ev.data.queue.queue = queue;
snd_seq_event_output(seq, &ev);
snd_seq_drain_output(seq);

  snd_seq_close(seq);

  return 0;
  }
