trait Tick {
  var amp: Double
  val srate = 100
  def tick(): Double

  def print {
    val values = 0.until(srate)
      .map(x => tick)
      .map(v => BigDecimal(v).setScale(4, BigDecimal.RoundingMode.HALF_UP))
    println("{" + values.mkString(",") + "}")
  }

  def print2 {
    val values = 0.until(srate)
      .map(x => tick)
      .map(v => BigDecimal(v).setScale(4, BigDecimal.RoundingMode.HALF_UP))
    println(values.mkString(" "))
  }
}

case class SineWave(var freq: Double, var amp: Double) extends Tick {

  var phase: Double = 0;

  def tick() = {
    val p_inc: Double = freq / srate;
    val ret = amp * math.sin(2 * math.Pi * phase);
    phase += p_inc;
    phase -= phase.toInt;
    ret
  }
}

object Additive {
  var maxFreq = 5
  var additionals = 15
}

class Additive extends Tick {
  var elements: List[Tick] = List(SineWave(1, 0.5))

  randomize
  normalize

  def randomize() {
    for (i <- 1 to Additive.additionals) {
      elements ::= SineWave(math.random * Additive.maxFreq, math.random)
    }
  }

  def normalize() {
    elements.foreach(x => x.amp /= amp)
  }

  def tick = elements.map(_.tick).sum

  def amp = elements.map(_.amp).sum
  def amp_=(newAmp: Double) {
    val oldAmp = amp
    elements.foreach(x => x.amp *= (newAmp / oldAmp))
  }
}




