input {
    ticksperbeat: number
    gate: logic
    time: virtual_cv
  }

output {
    pitch: virtual_cv
    data: array(4) of {
	value: virtual_cv
    }
    gate: logic
  }
