#include "bp_node.h"


int	receive_bundle(char *ownEid, char *buffer, size_t bufflen)
{
	BpSAP	sap = NULL;
	Sdr		sdr;
	BpDelivery	dlv;
	int		contentLength;
	ZcoReader	reader;
	int		len;

#ifndef mingw
	setlinebuf(stdout);
#endif
	if (ownEid == NULL)
	{
        putErrmsg("ERROR: endpointID missing", ownEid);
		return -1;
	}

	if (bp_attach() < 0)
	{
		putErrmsg("Can't attach to BP.", NULL);
		return -1;
	}

	if (bp_open(ownEid, &sap) < 0)
	{
		putErrmsg("Can't open own endpoint.", ownEid);
		return -1;
	}


	sdr = bp_get_sdr();

	if (bp_receive(sap, &dlv, BP_POLL) < 0)
	{
		putErrmsg("bpsink bundle reception failed.", NULL);
	}

    if (dlv.result == BpPayloadPresent)
    {
        CHKZERO(sdr_begin_xn(sdr));
        contentLength = zco_source_data_length(sdr, dlv.adu);
        sdr_exit_xn(sdr);
        if (contentLength < bufflen)
        {
            zco_start_receiving(dlv.adu, &reader);
            CHKZERO(sdr_begin_xn(sdr));
            len = zco_receive_source(sdr, &reader,
                    contentLength, buffer);
            if (sdr_end_xn(sdr) < 0 || len < 0)
            {
                putErrmsg("Can't handle delivery.", NULL);
                return -1;
            }
            buffer[contentLength] = '\0';

        }
    }
	else if (dlv.result == BpReceptionTimedOut)
	{
		// Timeout, just continue and check again when the thread calls receive again
		contentLength = 0;
	}
    else
    {
        putErrmsg("dlv.result is != BpPayloadPresent", NULL);
    }
    bp_release_delivery(&dlv, 1);

	bp_close(sap);
	writeErrmsgMemos();
	bp_detach();
	return contentLength;
}

static ReqAttendant	*_attendant(ReqAttendant *newAttendant)
{
	static ReqAttendant	*attendant = NULL;

	if (newAttendant)
	{
		attendant = newAttendant;
	}

	return attendant;
}


int	send_bundle(char *sourceEid, char *destEid, char *text, int textlen, int ttl)
{
	if(!ttl)
	{
		ttl = DEFAULT_TTL;
	}

	Sdr sdr;
	// BpSAP *ionsapPtr;
	Object extent;
	Object bundleZco;
	ReqAttendant attendant;
	Object newBundle;

	// newBundle = malloc(1000);
	if (sourceEid == NULL || destEid == NULL || ttl <= 0)
	{
		printf("ERROR: Provide a non-NULL source/destiny EID and a TTL>0\n");
		return 0;
	}

	if (bp_attach() < 0)
	{
		printf("ERROR when attaching to BP\n");
		return 0;
	}

	// if (bp_open_source(sourceEid, ionsapPtr, 0) < 0)
	// {
	// 	printf("ERROR when opening BP endpoin %s\n", sourceEid);
	// 	return 0;
	// }

	if (ionStartAttendant(&attendant))
	{
		printf("Cannot start blocking transmission\n");
		return 0;
	}

	_attendant(&attendant);
	sdr = bp_get_sdr();
	if (text)
	{

		if (textlen <= 0)
		{
			printf("Legth of the msg must be greater than 0");
			bp_detach();
			return 0;
		}

		CHKZERO(sdr_begin_xn(sdr));
		extent = sdr_malloc(sdr, textlen);
		if (extent)
		{
			sdr_write(sdr, extent, text, textlen);
		}

		if (sdr_end_xn(sdr) < 0)
		{
			printf("No space for ZCO extent.");
			bp_detach();
			return 0;
		}

		bundleZco = ionCreateZco(ZcoSdrSource, extent, 0, textlen,
				BP_STD_PRIORITY, 0, ZcoOutbound, &attendant);
		if (bundleZco == 0 || bundleZco == (Object) ERROR)
		{
			putErrmsg("Can't create ZCO extent.", NULL);
			bp_detach();
			return 0;
		}
		if (bp_send(NULL, destEid, NULL, ttl, BP_STD_PRIORITY,
				NoCustodyRequested, 0, 0, NULL, bundleZco,
				&newBundle) < 1)
		{
			putErrmsg("bpsource can't send ADU.", NULL);
		}
		// bp_close(ionsapPtr);
		ionStopAttendant(_attendant(NULL));
		bp_detach();
		return 0;
	}
	writeMemo("[i] Stopping bpsource.");
	ionStopAttendant(_attendant(NULL));
	bp_detach();
	return 0;
}