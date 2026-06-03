    float JumpscareHearingRadius(float scale = 1.0f) const {
        return std::max(0.1f, gameWorld_.maze.TileAverage()) * 4.85f * std::max(0.1f, scale);
    }

    float LightBulbBreakHearingRadius() const {
        return TileHearingRadius(28.0f);
    }

    float FlashlightClickHearingRadius() const {
        return std::max(0.55f, gameWorld_.maze.TileMinimum() * 0.62f);
    }

    float AirVentHearingRadius() const {
        return TileHearingRadius(5.0f);
    }

    float SparkHearingRadius(float intensity = 1.0f) const {
        return TileHearingRadius(Lerp(9.0f, 16.0f, Clamp01(intensity / std::max(0.1f, settingsRuntime_.live.effectBrokenLampSparkIntensityMax))));
    }
