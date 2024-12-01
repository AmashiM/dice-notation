using DiceNotationCS;

public class Program {
    public static void Main(string[] args)
    {
        DiceNotation notation = new DiceNotation();

        notation.SetText("1 + (1 + 1)");
        int retvalue = notation.Process();
        Console.WriteLine(retvalue);

        // long[] values = notation.Run();
        long value = notation.RunDefaultGroup();

        Console.WriteLine("got result: {0}", value);

        // Console.WriteLine(string.Format("[{0}]", string.Join(", ", values)));
    }
}