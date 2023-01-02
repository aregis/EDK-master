using huestream;

namespace HueStreamExampleWPF
{
    class BrightnessSawEffect : AreaEffect
    {
        public BrightnessSawEffect(string name, uint priority) : base(name, priority)
        {
        }

        public void PrepareEffect()
        {
            var r =  new SequenceAnimation(double.MaxValue);

            var rampUp = new TweenAnimation(0, 1, 1000, TweenType.EaseInOutQuad);
            r.Append(rampUp, "Up");

            var rampDown = new TweenAnimation(1, 0, 1000, TweenType.EaseInOutQuad);
            r.Append(rampDown, "Down");

            var g = new ConstantAnimation(0);
            var b = new ConstantAnimation(0);

            SetColorAnimation(r, g, b);

            AddArea(Area.All);
        }
    }
}