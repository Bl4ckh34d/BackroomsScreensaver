    static XMFLOAT3 TransformInstancePoint(XMFLOAT3 origin,
                                           XMFLOAT3 axisX,
                                           XMFLOAT3 axisY,
                                           XMFLOAT3 axisZ,
                                           XMFLOAT3 p) {
        return Add3(origin, Add3(Scale3(axisX, p.x), Add3(Scale3(axisY, p.y), Scale3(axisZ, p.z))));
    }
