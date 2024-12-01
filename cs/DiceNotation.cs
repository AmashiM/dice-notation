using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using static System.Runtime.InteropServices.JavaScript.JSType;

namespace DiceNotationCS
{
    enum TokenType {
        Empty,
        Dice,
        Number,
        Keep,
        KeepLow,
        KeepHigh,
        Group,
        GroupStart,
        GroupEnd,
        Add,
        Sub,
        Mult,
        Div,
        Unexpected
    };

    internal class DiceUtil
    {
        public static string TokenTypeAsString(TokenType type)
        {
            switch (type)
            {
                case TokenType.Empty: return "EMPTY";
                case TokenType.Dice: return "DICE";
                case TokenType.Number: return "NUM";
                case TokenType.Keep: return "KEEP";
                case TokenType.KeepLow: return "KEEP_LOW";
                case TokenType.KeepHigh: return "KEEP_HIGH";
                case TokenType.Group: return "GROUP";
                case TokenType.GroupStart: return "GROUP_START";
                case TokenType.GroupEnd: return "GROUP_END";
                case TokenType.Add: return "ADD";
                case TokenType.Sub: return "SUB";
                case TokenType.Mult: return "MULT";
                case TokenType.Div: return "DIV";
                case TokenType.Unexpected: return "UNEXPECTED";
                default: return "UNKNOWN";
            }
        }

        public static string RangeTokenTypeAsString(RangeTokenType type)
        {
            switch (type)
            {
                case RangeTokenType.Number: return "Number";
                case RangeTokenType.Dice: return "Dice";
                //case RangeTokenType.Complex: return "Complex";
                case RangeTokenType.Group: return "Group";
                case RangeTokenType.Keep: return "Keep";
                case RangeTokenType.Add: return "Add";
                case RangeTokenType.Sub: return "Sub";
                case RangeTokenType.Mult: return "Mult";
                case RangeTokenType.Div: return "Div";
                default: return "UNKNOWN";
            }
        }

        public static string FormatArray<T>(T[] array)
        {
            return string.Format("[{0}]", string.Join(", ", array));
        }
    }

    internal struct PairToken {
        public TokenType type;
        public int value;
        public bool claimed;
        public int range_pos;
        public int internal_pos;

        public override readonly string ToString()
        {
            return string.Format("PairToken(type: {0}, value: {1}, claimed: {2}, internal_pos: {3})", DiceUtil.TokenTypeAsString(type), value, claimed, internal_pos);
        }
    };

    internal struct DiceToken
    {
        // these values are all indexes
        public int pos;
        public int amount;
        public int sides;
        public int keep_high;
        public int keep_low;


        public override readonly string ToString()
        {
            return string.Format("DiceToken(pos: {0}, amount: {1}, sides: {2}, keep_high: {3}, keep_low: {4})", pos, amount, sides, keep_high, keep_low);
        }
    }

    enum RangeTokenType
    {
        Number,
        Dice,
        Keep,
        Add,
        Sub,
        Mult,
        Div,
        Group
    };

    internal struct RangeToken
    {
        public RangeTokenType type;
        public int start;
        public int end;
        public int main;


        public override readonly string ToString()
        {
            return string.Format("RangeToken(start: {0}, end: {1}, main: {2}, type: {3})", start, end, main, DiceUtil.RangeTokenTypeAsString(type));
        }
    }

    internal struct GroupPlacement
    {
        public int pos;
        public int priority;
        public bool claimed;
        public int group;

        public override readonly string ToString()
        {
            return string.Format("GroupPlacement(pos: {0}, priority: {1}, claimed: {2}, group: {3})", pos, priority, claimed, group);
        }
    }

    internal struct Group
    {
        public int start;
        public int end;
        public int priority;

        public int[] unclaimed;

        public override readonly string ToString()
        {
            return string.Format("Group(start: {0}, end: {1}, priority: {2}, unclaimed: {3})", start, end, priority, DiceUtil.FormatArray(unclaimed));
        }
    }

    internal class DiceCache
    {
        public PairToken[] tokens;
        public RangeToken[] ranges;
        public DiceToken[] dice_tokens;

        // the following is only meant to be indexes to the real values;
        public int[] numbers;
        public int[] dice;
        public int[] keep;
        public int[] add;
        public int[] sub;
        public int[] mult;
        public int[] div;

        public GroupPlacement[] group_start;
        public GroupPlacement[] group_end;

        public Group[] group;

        public DiceCache() {
            tokens = new PairToken[1];
            ranges = new RangeToken[1];
            dice_tokens = new DiceToken[1];
            numbers = new int[1];
            dice = new int[1];
            keep = new int[1];
            add = new int[1];
            sub = new int[1];
            div = new int[1];
            mult = new int[1];
            group_start = new GroupPlacement[1];
            group_end = new GroupPlacement[1];

            group = new Group[1];
        }

        public void SetGroupClaimed(int index, bool claimed)
        {
            Group temp = group[index];
            tokens[temp.start].claimed = claimed;
            if(temp.end != -1 && temp.end < tokens.Length)
            {
                tokens[temp.end].claimed = claimed;
            }
        }

        public void PrintGroups()
        {
            for(int i = 0; i < group.Length; i++) {
                Console.WriteLine(group[i].ToString());
            }
        }

        public void PrintTokens()
        {
            for(int i = 0; i < tokens.Length; i++)
            {
                Console.WriteLine(tokens[i].ToString());
            }
        }

        public void PrintIndexes()
        {
            Console.WriteLine("numbers: " + DiceUtil.FormatArray(numbers));
            Console.WriteLine("dice: " + DiceUtil.FormatArray(dice));
            Console.WriteLine("keep: " + DiceUtil.FormatArray(keep));
            Console.WriteLine("add: " + DiceUtil.FormatArray(add));
            Console.WriteLine("sub: " + DiceUtil.FormatArray(sub));
            Console.WriteLine("div: " + DiceUtil.FormatArray(div));
            Console.WriteLine("mult: " + DiceUtil.FormatArray(mult));
            Console.WriteLine("group_start: " + DiceUtil.FormatArray(group_start));
            Console.WriteLine("group_end: " + DiceUtil.FormatArray(group_end));
            Console.WriteLine("groups: " + DiceUtil.FormatArray(group));
        }

        public int[] GetUnclaimed()
        {
            int[] claims = new int[tokens.Length];

            int total_unclaimed = 0;
            for(int i = 0; i < tokens.Length; i++)
            {
                if (tokens[i].claimed)
                {

                } else
                {
                    claims[total_unclaimed] = i;
                    total_unclaimed++;
                }
            }
            int[] new_claims = new int[total_unclaimed];
            for(int i = 0; i < total_unclaimed; i++)
            {
                new_claims[i] = claims[i];
            }
            return new_claims;
        }
    }

    internal class DiceCounters {
        public int length;
        public int real_token_count;

        public int number_count;
        public int dice_count;
        public int add_count;
        public int sub_count;
        public int mult_count;
        public int div_count;
        public int keep_count;
        public int group_start_count;
        public int group_end_count;

        public int group_count;

        public int EstimateRangeTokenCount()
        {
            return number_count + dice_count + add_count + sub_count + mult_count + div_count + keep_count;
        }
    }

    internal class DiceNotation
    {
        private readonly DiceCache cache;
        private readonly DiceCounters counters;
        private string text;
        private readonly Random rnd;

        public DiceNotation() {
            cache = new DiceCache();
            counters = new DiceCounters();
            rnd = new Random(DateTime.Now.Millisecond);
            text = "";
        }

        public void SetText(string text)
        {
            this.text = text;
            counters.length = text.Length;
        }

        public int ProcessText()
        {
            int real_token_count = counters.length;

            cache.tokens = new PairToken[counters.length];

            bool processing_number = false;
            int last_number_pos = 0;

            for(int i = 0; i < counters.length; i++)
            {
                char c = text[i];
                TokenType type;
                int value = 0;

                if('0' <= c && c <= '9')
                {
                    type = TokenType.Number;
                    value = c - '0';
                } else
                {
                    switch (c)
                    {
                        case ' ':
                            type = TokenType.Empty; break;
                        case 'd':
                            counters.dice_count++;
                            type = TokenType.Dice; break;
                        case '+':
                            counters.add_count++;
                            type = TokenType.Add; break;
                        case '-':
                            counters.sub_count++;
                            type = TokenType.Sub; break;
                        case '*':
                            counters.mult_count++;
                            type = TokenType.Mult; break;
                        case '/':
                            counters.div_count++;
                            type = TokenType.Div; break;
                        case '(':
                            counters.group_start_count++;
                            type = TokenType.GroupStart; break;
                        case ')':
                            counters.group_end_count++;
                            type = TokenType.GroupEnd; break;
                        case 'k': {
                            char next_c = text[i+1];
                            switch(next_c)
                            {
                                case 'l': {
                                    type = TokenType.KeepLow;
                                }; break;
                                case 'h': {
                                    type = TokenType.KeepHigh;
                                }; break;
                                default: {
                                    type = TokenType.Keep;        
                                }; break;
                            }
                            counters.keep_count++;
                        }; break;
                        default: {
                            type = TokenType.Unexpected;
                        }; break;
                    }
                }

                if(type == TokenType.Number)
                {
                    if (processing_number)
                    {
                        type = TokenType.Empty;
                        cache.tokens[last_number_pos].value = (cache.tokens[last_number_pos].value * 10) + value;
                        value = 0;
                    } else
                    {
                        counters.number_count++;
                        processing_number = true;
                        last_number_pos = i;
                    }
                } else
                {
                    if (processing_number)
                    {
                        processing_number = false;
                    }
                }

                if(type == TokenType.Empty)
                {
                    real_token_count--;
                }

                cache.tokens[i].type = type;
                cache.tokens[i].value = value;
                cache.tokens[i].claimed = false;
            }

            counters.real_token_count = real_token_count;

            return 0;
        }

        public int ProcessTokens()
        {
            PairToken[] new_tokens = new PairToken[counters.real_token_count];
            int new_token_pos = 0;

            for(int i = 0; i < counters.length; i++)
            {
                PairToken token = cache.tokens[i];
                if(token.type == TokenType.Empty)
                {
                    continue;
                }
                new_tokens[new_token_pos] = token;

                new_token_pos++;
            }

            cache.tokens = new_tokens;

            return 0;
        }

        public int ProcessTokenIndexes()
        {
            cache.numbers = new int[counters.number_count];
            cache.dice = new int[counters.dice_count];
            cache.keep = new int[counters.keep_count];
            cache.add = new int[counters.add_count];
            cache.sub = new int[counters.sub_count];
            cache.mult = new int[counters.mult_count];
            cache.div = new int[counters.div_count];
            cache.group_start = new GroupPlacement[counters.group_start_count];
            cache.group_end = new GroupPlacement[counters.group_end_count];

            int number_pos = 0;
            int dice_pos = 0;
            int keep_pos = 0;
            int add_pos = 0;
            int sub_pos = 0;
            int mult_pos = 0;
            int div_pos = 0;

            int group_start_pos = 0;
            int group_end_pos = 0;

            int group_priority = 0;

            for(int i = 0; i < cache.tokens.Length; i++)
            {
                PairToken token = (PairToken)cache.tokens[i];
                switch(token.type)
                {
                    case TokenType.Number:
                        token.internal_pos = number_pos;
                        cache.numbers[number_pos] = i;
                        number_pos++;
                        break;
                    case TokenType.Dice:
                        token.internal_pos = dice_pos;
                        cache.dice[dice_pos] = i;
                        dice_pos++;
                        break;
                    case TokenType.Keep:
                        token.internal_pos = keep_pos;
                        cache.keep[keep_pos] = i;
                        keep_pos++;
                        break;
                    case TokenType.Add:
                        token.internal_pos = add_pos;
                        cache.add[add_pos] = i;
                        add_pos++;
                        break;
                    case TokenType.Sub:
                        token.internal_pos = sub_pos;
                        cache.sub[sub_pos] = i;
                        sub_pos++;
                        break;
                    case TokenType.Div:
                        token.internal_pos = div_pos;
                        cache.div[div_pos] = i;
                        div_pos++;
                        break;
                    case TokenType.Mult:
                        token.internal_pos = mult_pos;
                        cache.mult[mult_pos] = i;
                        mult_pos++;
                        break;
                    case TokenType.GroupStart:
                        {
                            token.internal_pos = group_start_pos;
                            group_priority++;
                            GroupPlacement groupPlacement;
                            groupPlacement.pos = i;
                            groupPlacement.priority = group_priority;
                            groupPlacement.group = -1;
                            groupPlacement.claimed = false;
                            cache.group_start[group_start_pos] = groupPlacement;
                        }; break;
                    case TokenType.GroupEnd:
                        {
                            token.internal_pos = group_end_pos;
                            GroupPlacement groupPlacement;
                            groupPlacement.pos = i;
                            groupPlacement.priority = group_priority;
                            groupPlacement.group = -1;
                            groupPlacement.claimed = false;
                            cache.group_end[group_end_pos] = groupPlacement;
                            group_priority--;
                        }; break;
                    default:
                        Console.WriteLine("unhandled token storage: {0}", DiceUtil.TokenTypeAsString(token.type));
                        break;
                }
                cache.tokens[i] = token;
            }

            return 0;
        }

        public int DefineGroups()
        {
            if(counters.group_start_count != counters.group_end_count)
            {
                Console.WriteLine("there needs to be an equal amount of group start tokens and group end tokens");
                return 1;
            }
            counters.group_count = counters.group_start_count + 1;

            cache.group = new Group[counters.group_count];

            Group defaultGroup;
            defaultGroup.start = 0;
            defaultGroup.end = counters.real_token_count;
            defaultGroup.priority = 0;
            defaultGroup.unclaimed = new int[1];

            Console.WriteLine("default Group: {0}", defaultGroup);

            cache.group[0] = defaultGroup;

            for(int i = 0; i < counters.group_start_count; i++)
            {
                Group group;
                group.unclaimed = new int[1];
                GroupPlacement startPlacement = cache.group_start[i];
                group.priority = startPlacement.priority;
                group.start = startPlacement.pos;
                group.end = -1;
                for(int j = 0; j < counters.group_end_count; j++)
                {
                    GroupPlacement endPlacement = cache.group_end[j];
                    Console.WriteLine("start: {0}", startPlacement.ToString());
                    Console.WriteLine("end: {0}", endPlacement.ToString());
                    if (endPlacement.claimed)
                    {
                        continue;
                    }
                    if(endPlacement.priority != startPlacement.priority)
                    {
                        continue;
                    }
                    if(endPlacement.pos < startPlacement.pos)
                    {
                        continue;
                    }
                    group.end = endPlacement.pos;
                    endPlacement.group = i + 1;
                    cache.group_end[j] = endPlacement;
                    break;
                }
                startPlacement.group = i+1;
                cache.group_start[i] = startPlacement;
                cache.group[i+1] = group;
                cache.SetGroupClaimed(i, true);
            }


            return 0;
        }

        public int OrganizeGroups()
        {
            for (int i = 0; i < cache.group.Length; i++)
            {
                Group i_group = cache.group[i];
                for(int j = i+1; j < cache.group.Length; j++)
                {
                    Group j_group = cache.group[j];
                    bool swap = false;
                    if(i_group.priority < j_group.priority)
                    {
                        swap = true;
                    } else if(i_group.priority == j_group.priority)
                    {
                        if(i_group.start > j_group.start)
                        {
                            swap = true;
                        }
                    }

                    if (swap)
                    {
                        cache.group[i] = j_group;
                        cache.group[j] = i_group;
                    }
                }
            }
            return 0;
        }
        public int DefineRanges()
        {
            int range_pos = 0;

            cache.ranges = new RangeToken[counters.EstimateRangeTokenCount()];

            for(int i = 0; i < counters.number_count; i++)
            {
                int pos = cache.numbers[i];
                RangeToken token;
                token.main = pos;

                token.start = pos;
                token.end = pos;
                token.type = RangeTokenType.Number;

                cache.ranges[range_pos] = token;
                cache.tokens[pos].range_pos = range_pos;
                range_pos++;
            }
            
            for(int i = 0; i < counters.keep_count; i++)
            {
                int pos = cache.keep[i];
                RangeToken token;
                token.main = pos;

                token.start = pos;
                PairToken pair_token = cache.tokens[pos+1];
                if(pair_token.type == TokenType.Number)
                {
                    token.end = pos + 1;
                    cache.tokens[pos+1].claimed = true;
                } else
                {
                    token.end = pos;
                }
                token.type = RangeTokenType.Keep;
                cache.ranges[range_pos] = token;
                cache.tokens[pos].range_pos = range_pos;
                range_pos++;
            }

            for (int i = 0; i < counters.dice_count; i++)
            {
                int pos = cache.dice[i];

                RangeToken token;
                token.main = pos;

                token.start = pos-1;
                token.end = pos+1;
                token.type = RangeTokenType.Dice;

                cache.tokens[pos - 1].claimed = true;
                cache.tokens[pos + 1].claimed = true;
                cache.ranges[range_pos] = token;
                cache.tokens[pos].range_pos = range_pos;
                range_pos++;
            }

            for (int i = 0; i < counters.add_count; i++)
            {
                int pos = cache.add[i];
                RangeToken token;
                token.main = pos;

                token.start = pos;
                for (int j = pos - 1; j > -1; j--)
                {
                    PairToken pair_token = cache.tokens[j];
                    if (!pair_token.claimed)
                    {
                        token.start = j;
                        break;
                    }
                }

                token.end = pos;
                for(int j = pos + 1; j < counters.length; j++)
                {
                    PairToken pair_token = cache.tokens[j];
                    if (!pair_token.claimed)
                    {
                        token.end = j;
                        break;
                    }
                }

                if(token.start != pos)
                {
                    cache.tokens[token.start].claimed = true;
                }

                if(token.end != pos)
                {
                    cache.tokens[token.end].claimed = true;
                }

                token.type = RangeTokenType.Add;

                cache.ranges[range_pos] = token;
                cache.tokens[pos].range_pos = range_pos;
                range_pos++;
            }

            return 0;
        }

        public int DefineDice()
        {
            cache.dice_tokens = new DiceToken[counters.dice_count];

            for(int i = 0; i < counters.dice_count; i++)
            {
                DiceToken dice_token;

                int pos = cache.dice[i];
                dice_token.pos = pos;

                dice_token.amount = -1;
                dice_token.sides = -1;
                dice_token.keep_low = -1;
                dice_token.keep_high = -1;

                PairToken token = cache.tokens[pos];
                cache.tokens[pos].value = i;
                RangeToken range = cache.ranges[token.range_pos];
                bool processing_after = false;

                int dice_range_size = range.end - range.start;

                Console.WriteLine("dice range size: {0}", dice_range_size);

                for(int j = range.start; j < range.end+1; j++)
                {
                    if(j == pos)
                    {
                        processing_after = true;
                        continue;
                    }
                    PairToken pairToken = cache.tokens[j];

                    Console.WriteLine("defining dice, got token: {0}", pairToken);

                    switch(pairToken.type)
                    {
                        case TokenType.Number:
                            if (processing_after)
                            {
                                if(dice_token.sides == -1){
                                    dice_token.sides = pairToken.range_pos;
                                }
                            } else
                            {
                                if(dice_token.amount == -1){
                                    dice_token.amount = pairToken.range_pos;
                                }
                            }
                            break;
                        case TokenType.KeepLow:
                            dice_token.keep_low = pairToken.range_pos;
                            j++;
                            break;
                        case TokenType.KeepHigh:
                            dice_token.keep_high = pairToken.range_pos;
                            j++;
                            break;
                        default:
                            Console.WriteLine(string.Format("unexpected token type when defining dice: {0}", DiceUtil.TokenTypeAsString(pairToken.type)));
                            break;
                    }
                }

                Console.WriteLine("got new dice: {0}", dice_token);

                cache.dice_tokens[i] = dice_token;
            }

            return 0;
        }


        public int AssignUnclaimedToGroups(){
            for(int i = (counters.group_count-1); 0 <= i; i--){
                Console.WriteLine("started claiming group");
                Group group = cache.group[i];
                int size = group.end - group.start;
                Console.WriteLine("Group: {1}\nGroup size: {0}", size, group.ToString());
                int[] claims = new int[size];
                int total_unclaimed = 0;
                for(int j = group.start+1; j < group.end; j++){
                    if(cache.tokens.Length <= j || j < 0){
                        Console.WriteLine("Attempt to access index outside of token cache");
                        return 1;
                    }
                    PairToken token = cache.tokens[j];

                    Console.WriteLine("pos: {0}, type: {1}, is claimed: {2}", j, token.type, token.claimed);

                    switch(token.type){
                        case TokenType.GroupStart:
                            Group temp_group = cache.group[cache.group_start[token.internal_pos].group];
                            j = temp_group.end;
                            break;
                        case TokenType.GroupEnd:
                            break;
                        default: {
                            if(!token.claimed){
                                Console.WriteLine("claiming");
                                claims[total_unclaimed] = j;
                                total_unclaimed++;
                                cache.tokens[j].claimed = true;
                            }
                        }; break;
                    }
                }
                group.unclaimed = new int[total_unclaimed];
                for(int j = 0; j < total_unclaimed; j++){
                    group.unclaimed[j] = claims[j];
                }
                cache.group[i] = group;
            }
            return 0;
        }

        public int Process()
        {
            int retvalue;

            retvalue = ProcessText();
            if(retvalue != 0)
            {
                Console.WriteLine("error when processing text");
                return 1;
            }

            retvalue = ProcessTokens();
            if(retvalue != 0)
            {
                Console.WriteLine("error when processing tokens");
                return 1;
            }

            retvalue = ProcessTokenIndexes();
            if(retvalue != 0)
            {
                Console.WriteLine("error when assigning token indexes");
                return 1;
            }

            cache.PrintTokens();

            Console.WriteLine("---");

            retvalue = DefineGroups();
            if(retvalue != 0)
            {
                Console.WriteLine("error defining groups");
                return 1;
            }

            retvalue = OrganizeGroups();
            if (retvalue != 0)
            {
                Console.WriteLine("error organizing groups");
                return 1;
            }

            retvalue = DefineRanges();
            if(retvalue != 0)
            {
                Console.WriteLine("error defining ranges");
                return 1;
            }

            retvalue = AssignUnclaimedToGroups();
            if(retvalue != 0)
            {
                Console.WriteLine("error assigning unclaimed to groups");
                return 1;
            }

            cache.PrintGroups();

            cache.PrintTokens();

            cache.PrintIndexes();

            retvalue = DefineDice();
            if(retvalue != 0)
            {
                Console.WriteLine("failed to define dice");
                return 1;
            }

            Console.WriteLine(string.Format("[{0}]", string.Join(", ", cache.ranges)));

            Console.WriteLine(string.Format("[{0}]", string.Join(", ", cache.dice_tokens)));

            Console.WriteLine(DiceUtil.FormatArray(cache.GetUnclaimed()));

            return 0;
        }

        internal enum RangeResultType
        {
            Number,
            Keep
        }

        internal struct RangeResult {
            public RangeResultType type;
            public long value;
        }

        public RangeResult RunDice(DiceToken token, long elevation)
        {
            RangeResult result;
            result.type = RangeResultType.Number;
            long out_value = 0;

            long amount;
            long sides;
            //bool uses_keep = false;
            //long keep_high = 0;
            //long keep_low = 0;

            // parse

            if(token.amount == -1)
            {
                amount = 1;
            } else
            {
                RangeToken amountRange = cache.ranges[token.amount];
                RangeResult amountResult = RunRange(amountRange, elevation+1);
                if(amountResult.type == RangeResultType.Number)
                {
                    amount = amountResult.value;
                } else
                {
                    Console.WriteLine("unknown result type recieved for dice amount");
                    amount = 1;
                }
            }

            if(token.sides == -1)
            {
                sides = 4;
            } else
            {
                RangeToken sidesRange = cache.ranges[token.sides];
                Console.WriteLine("got side range: {0}", sidesRange);
                RangeResult sidesResult = RunRange(sidesRange, elevation+1);
                if (sidesResult.type == RangeResultType.Number)
                {
                    sides = sidesResult.value;
                }
                else
                {
                    Console.WriteLine("unknown result type recieved for dice sides");
                    sides = 4;
                }
            }

            // roll the dice
            int[] values = new int[amount];

            for(int i = 0; i < (int)amount; i++)
            {
                int value = rnd.Next(1, (int)sides+1);
                Console.WriteLine("rolling dice [{3}] {1}d{2}: {0}", value, amount, sides, token.pos);
                values[i] = value;
            }

            for(int i = 0; i < amount; i++)
            {
                out_value = out_value + values[i];
            }

            Console.WriteLine("got dice result: {0}", out_value);

            result.value = out_value;
            return result;
        }

        public RangeResult RunGroup(Group group, long elevation){
            

            if(group.unclaimed.Length > 1 || group.unclaimed.Length == 0) {
                RangeResult finalResult;
                Console.WriteLine("unable to handle groups with mutltiple return values or no return values");
                finalResult.type = RangeResultType.Number;
                finalResult.value = 0;
                return finalResult;
            }

            int unclaimed = group.unclaimed[0];
            PairToken token = cache.tokens[unclaimed];
            return RunRange(cache.ranges[token.range_pos], elevation+1);
        }

        public RangeResult RunRange(RangeToken token, long elevation)
        {
            RangeResult result;
            long out_value = 0;

            result.type = RangeResultType.Number;

            Console.WriteLine("({1}) Running Range: {0}", DiceUtil.RangeTokenTypeAsString(token.type), elevation);

            switch(token.type)
            {
                case RangeTokenType.Number:
                    out_value = cache.tokens[token.main].value;
                    break;
                case RangeTokenType.Add:
                    RangeToken start = cache.ranges[cache.tokens[token.start].range_pos];
                    RangeToken end = cache.ranges[cache.tokens[token.end].range_pos];
                    RangeResult start_value = RunRange(start, elevation+1);
                    RangeResult end_value = RunRange(end, elevation+1);
                    out_value = start_value.value + end_value.value;
                    break;
                case RangeTokenType.Dice: {
                        DiceToken diceToken = cache.dice_tokens[cache.tokens[token.main].value];
                        Console.WriteLine("running range got dice token: {0}", diceToken);
                        
                        RangeResult diceResult = RunDice(diceToken, elevation + 1);

                        if(diceResult.type == RangeResultType.Number)
                        {
                            out_value = diceResult.value;
                        } else
                        {
                            Console.WriteLine("unexpected value from dice result");
                        }
                }; break;
                case RangeTokenType.Group:
                    Group group = cache.group[token.main];
                    RangeResult groupResult = RunGroup(group, elevation + 1);
                    return groupResult;
            }

            result.value = out_value;

            return result;
        }

        private long[] RunUnclaimed(int[] unclaimed)
        {
            long[] values = new long[unclaimed.Length];

            for (int i = 0; i < unclaimed.Length; i++)
            {
                int pos = (int)unclaimed[i];
                PairToken token = cache.tokens[pos];
                RangeResult result = RunRange(cache.ranges[token.range_pos], 0);

                if (result.type == RangeResultType.Number)
                {
                    values[i] = result.value;
                }
                else
                {
                    values[i] = 0;
                }
            }

            return values;
        }

        public long[] Run()
        {
            int[] unclaimed = cache.GetUnclaimed();

            return RunUnclaimed(unclaimed);
        }

        public long RunDefaultGroup(){
            Group defaultGroup = cache.group[0];
            RangeResult result = RunGroup(defaultGroup, 0);
            return result.value;
        }
    }
}
