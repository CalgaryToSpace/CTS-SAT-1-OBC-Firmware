# GNSS Mission Operations

## Correlating GNSS data to a timestamp

All OEM7 ASCII responses contain the GPS Week and GPS Seconds fields in the header (e.g., the `log bestxyza once` command). Those fields can easily be converted to a timestamp/date.

```python
from datetime import datetime, timedelta, timezone

def gps_to_datetime(gps_week: int, gps_seconds: float):
    gps_epoch = datetime(1980, 1, 6, tzinfo=timezone.utc)
    return gps_epoch + timedelta(weeks=gps_week, seconds=gps_seconds)
```

