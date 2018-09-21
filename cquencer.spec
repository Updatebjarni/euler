in {
    ticksperbeat: int32
    step: array(8) of {
      pitch: int32
      value1: int32
      value2: int32
    }
  }

out {
    pitch: int32
    value1: int32
    value2: int32
    gate: bool
  }
