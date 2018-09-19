in {
  foo: int32
  bar: array(10) of int32
  baz: { bat: int32 }
  chip: array(3) of {
    voice: array(3) of {
      pitch: int32
      gate: bool
      saw: bool
      triangle: bool
      pulse: bool
      noise: bool
      pw: int32
      attack: int32
      decay: int32
      sustain: int32
      release: int32
      filter: bool
      sync: bool
      ringmod: bool
      }
    vol: int32
    hp: bool
    bp: bool
    lp: bool
    res: int32
    cutoff: int32
    mute3: bool
    filtext: bool
    }
  mixer: int32
  }

out empty
