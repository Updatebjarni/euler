input {
    ticksperbeat: number
    gatelength: virtual_cv
    keys: key_events(44)
  }

output {
    pitch: virtual_cv
    acc: logic
    gate: logic
  }
