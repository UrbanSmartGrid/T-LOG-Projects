package com.example.testnfc;

import android.nfc.Tag;

class TLogData {
    private Tag     mTag;
    private byte[]  mUID;


    public TLogData(Tag aTag, byte[] aUID) {
        mTag = aTag;
        mUID = aUID;
    }

    public Tag getTag() {
        return mTag;
    }

    public void setTag(Tag tag) {
        mTag = tag;
    }

    public byte[] getUID() {
        return mUID;
    }

    public void setUID(byte[] UID) {
        mUID = UID;
    }
}

