/* $Id$ */

#include <sys/msg.h>
#include <sys/fs.h>
#include <sys/namer.h>

static void
helloworld_main(port_t rootport)
{
	struct msg m;
	int x;
	char statstr[] = "Hello World!\n";

loop:
	x = msg_receive(rootport, &m);
	if (x < 0) {
		goto loop;
	}

	switch (m.m_op & MSG_MASK) {

	case M_CONNECT:		/* New client */
		msg_accept(m.m_sender);
		break;

	case M_DISCONNECT:	/* Client done */
		break;

	case M_DUP:		/* File handle dup during exec() */
	case M_ABORT:		/* Aborted operation */
		m.m_nseg = m.m_arg = m.m_arg1 = 0;
		msg_reply(m.m_sender, &m);
		break;

	case FS_STAT:		/* Tell about file */
		m.m_nseg = 1;
		m.m_buf = statstr;
		m.m_buflen = strlen(statstr);
		m.m_arg = m.m_arg1 = 0;
		msg_reply(m.m_sender, &m);
		break;

	default:		/* Unknown */
		msg_err(m.m_sender, EINVAL);
		break;
	}
	goto loop;
}

void
main(int argc, char **argv)
{
	port_t rootport;
	port_name fsname;
	int x;

	rootport = msg_port(0, &fsname);
	namer_register("helloworld", fsname);

	helloworld_main(rootport);
}
