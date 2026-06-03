        if (!panicActive && !softStopActive && pathTurnTargetWeight > 0.001f) {
            if (cameraRuntime_.turnLookBlend < 0.012f) {
                cameraRuntime_.turnLookYaw = pathTurnTargetYaw;
            } else {
                cameraRuntime_.turnLookYaw += AngleWrap(pathTurnTargetYaw - cameraRuntime_.turnLookYaw) * std::min(1.0f, dt * 6.2f);
            }
            float response = pathTurnTargetWeight > cameraRuntime_.turnLookBlend
                ? (2.35f + pathTurnTargetWeight * 2.80f)
                : 5.8f;
            cameraRuntime_.turnLookBlend += (pathTurnTargetWeight - cameraRuntime_.turnLookBlend) * std::min(1.0f, dt * response);
        } else {
            cameraRuntime_.turnLookBlend += (0.0f - cameraRuntime_.turnLookBlend) * std::min(1.0f, dt * 5.2f);
            if (cameraRuntime_.turnLookBlend < 0.001f) {
                cameraRuntime_.turnLookBlend = 0.0f;
                cameraRuntime_.turnLookYaw = desiredYaw;
            }
        }
        pathTurnWeight = cameraRuntime_.turnLookBlend;
        if (!panicActive && !softStopActive && pathTurnWeight > 0.001f) {
            desiredYaw += AngleWrap(cameraRuntime_.turnLookYaw - desiredYaw) * pathTurnWeight;
        }
        XMFLOAT3 exitFocusTarget{};
        float exitLookTargetWeight = (!panicActive && !softStopActive) ? ExitAttentionWeight(exitFocusTarget) : 0.0f;
        if (exitLookTargetWeight > 0.001f) {
            viewRuntime_.exitLookFocus = viewRuntime_.exitLookBlend <= 0.001f
                ? exitFocusTarget
                : Lerp3(viewRuntime_.exitLookFocus, exitFocusTarget, std::min(1.0f, dt * 5.4f));
        }
        float exitLookResponse = exitLookTargetWeight > viewRuntime_.exitLookBlend
            ? (2.20f + exitLookTargetWeight * 2.75f)
            : 4.8f;
        viewRuntime_.exitLookBlend += (exitLookTargetWeight - viewRuntime_.exitLookBlend) * std::min(1.0f, dt * exitLookResponse);
        if (viewRuntime_.exitLookBlend < 0.001f) viewRuntime_.exitLookBlend = 0.0f;
