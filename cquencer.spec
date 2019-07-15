input {
    ticksperbeat: number
    step: array(8) of {
      pitch: virtual_cv
      value1: virtual_cv
      value2: virtual_cv
    }
  }

output {
    pitch: virtual_cv
    value1: virtual_cv
    value2: virtual_cv
    gate: logic
  }
