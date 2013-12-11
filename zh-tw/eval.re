= 程式與求值

== 什麼是程式語言

所謂的程式語言，就是用來撰寫程式用的語言。所謂的程式語言處理器，就是進行程式運算的地方，像是編譯器（compiler）和執行器的組合，或是直譯器（interpreter）。本書要實作的，是逐行解讀並執行程式的直譯器。直譯器會規範程式的行為，換個角度來看的話，也可以說直譯器是在對程式語言進行定義@<fn>{fn1}。

那麼用同樣的觀點來看，程式又是什麼呢？一般說法是「運算的步驟的紀錄」，但在本書的定義中，程式是「程式該如何被執行」的定義。我們這次會藉由 Ruby 的程式，而非由自然語言所寫成的規格，決定要如何接受並執行程式。藉此，我們就可以避開自然語言中曖昧不清的部分，對程式做出定義。

由於一開始很難理解全部的程式，因此我們先從簡單的例子開始，確定它能夠運作後再慢慢增加其他功能。

不要把「程式語言處理器」想得太難。總之它就是逐行讀入字串，並將之計算的程式。把它當成計算機的升級版本的話就輕鬆多了吧。我們的第一個例子，正是計算機。

== 第一次就上手的程式求值

首先，我們來看一下以下這段程式。

//emlist{
[:+, 1, 2]
//}

您應該會覺得奇怪，這是從來沒有看過的語法。那當然了。這是筆者自己想出來的 μSchemeR 程式語言（名字也是我自己取的）。您可能會覺得這個語法簡直是莫名其妙，但讀完本章之後，您應該就有能力可以把它修改成您喜歡的語法了，所以請先將就一下吧。

只要稍微動用一下想像力，就可以推測這或許是要把 1 和 2 相加的程式吧。正是如此。那麼，「請想一下，看到這段程式時，要如何處理才能得出這個結果」這個問題該如何回答呢？是該說「請給我 1 加 2 的結果」，還是「當句首是@<tt>{:+}時，請把緊接著的兩個參數的值相加」呢？

那麼，這段程式又該何處理呢？

//emlist{
[:+, [:+, 1, 2], 3]
//}

「先把@<tt>{[:+, 1, 2]}的結果計算出來，再把得出的答案和 3 加在一起」大概可以這樣說吧。您應該注意到了，我們在前一段的說明中，加進了「計算」這個詞。也就是說，當我們在思考要如何處理上述兩段程式，以取得其計算結果時，其處理方式就是要將緊接在@<tt>{:+}之後的參數予以「計算」後，再予以相加。這種計算，我們一般稱之為「求值（evaluation）。因此，「當句首是@<tt>{:+}時，對緊接的兩個參數求值，再將得出的值相加」就是我們想要的處理方式。

====[column]μSchemeR 的語法（syntax）

在這次設計的程式語言 μSchemeR 中，我們是把函數放在最前面，參數接在後面，例如這樣@<tt>{[:+, 1, 2]}。為什麼要用這種方式呢？

要像其他語言一樣能夠計算@<tt>{x + y * z}的話，就必須以運算元（operator）的優先順序，決定是要當作@<tt>{(x + y) * z}或@<tt>{x + (y * z)}。說得更進一步的話，之後會介紹的 if 也是像這樣，@<tt>{[:if, :true,  1, 0]}一定是在@<tt>{[}後面加上關鍵字@<tt>{:if}，因此只要看到這個字串，就能知道它是一般的函數套用，還是特別的語法。另一方面，如果要讓程式能夠處理形形色色的語法，例如@<tt>{if (true) then 1 else 2;やx = y + 1;}，那就必須要做更多的計算，亦即詞法分析（lexical analysis）、語法分析（syntactic analysis）。我們讓人類而不是電腦來處理這個問題，也是為了讓電腦能夠簡單地處理程式，並予以解釋（這不是要人類代替電腦做它應該做的工作，而是當程式語言的規格很單純時，人類在學習程式語言的規格時也會相對輕鬆。我們追求的是程式語言的易學性和程式寫作的方便性之間的平衡）。

另外，我們以「@<tt>{,}」區隔元素，前後加上「@<tt>{[}」と「@<tt>{]}」，作為 list 的表述方式。這種格式可以直接被 Ruby 當作 array 來處理，對電腦來說也是很方便的。另外，計算符號@<tt>{+}、變數@<tt>{x}、保留字串@<tt>{:if}等等，我們在前面加上@<tt>{:}，寫成@<tt>{:+}、@<tt>{:x}、@<tt>{:if}。這種寫法也是為了方便我們在 Ruby 中把它們直接當作 symbol 來用。

====[/column]

那麼，我們接著就來用 Ruby 把剛才的「取得程式結果的處理方式」寫出來吧。

@<tt>{_eval}@<fn>{fn2}是對表示式（expression） @<tt>{exp}求值，並回傳其結果。

表示式如果是串列（list）的話，就把一開始的元素當成函數，把剩下的當作參數，然後各自求值。把求得的參數套用（apply）在求得的函數中，所得到的結果就是@<tt>{_eval}的結果＠<fn>{fn3}。表示式不是 list 的話，如果是立即數（immediate number），就直接回傳數字。例如，@<tt>{2}就直接回傳 2。不是數字的話，就視為內建函數（built-in function），回傳被 Ruby 相聯（associate）的函數。

//emlist{
def _eval(exp)
  if not list?(exp) 
    if immediate_val?(exp)
      exp
    else 
      lookup_primitive_fun(exp)
    end
  else
    fun = _eval(car(exp))
    args = eval_list(cdr(exp))
    apply(fun, args)
  end
end
//}

//note[程式碼和值的區別]{
「@<tt>{2}會回傳 2」您注意到這段文字中的數字，字型是不一樣的嗎？@<tt>{2}是我們現在正在設計的 μSchemeR 程式，2 是用來表示（Ruby 中的）數值的 2 的。我們用這種體例來區別程式和它所求得的值。請參考下圖：@<img>{uschemer_ruby}
//}

接著，我們來說明上述 Ruby 程式所呼叫的參數。

我們從 instance 是不是陣列（array）來判斷它是不是 list。

//emlist{
def list?(exp)
  exp.is_a?(Array)
end
//}

built-in function 是以函數名稱為 key，將函數本身當作值 hash 起來存下。被 associate 的函數就是 Ruby 的函數。對 built-in function 求值，就會回傳被 associate 某個函數名稱的函數的值。

//emlist{
def lookup_primitive_fun(exp)
  $primitive_fun_env[exp]
end

$primitive_fun_env = {
  :+ => [:prim, lambda{|x, y| x + y}],
  :- => [:prim, lambda{|x, y| x - y}],
  :* => [:prim, lambda{|x, y| x * y}], 
}
//}

@<tt>{car}是 list 的第一個元素，@<tt>{cdr}是用來取得在那之後的 list 的函數。名字有點奇怪@<fn>{fn4}，不過這是 Scheme 使用的名字，所以請稍微忍耐一下。之後就會習慣了。

//emlist{
def car(list)
  list[0]
end

def cdr(list)
  list[1..-1]
end
//}

對參數求值的@<tt>{eval_list}，是用來將 list 的元素各自求值，再把它們做成 list。

//emlist{
def eval_list(exp)
  exp.map{|e| _eval(e)}
end    
//}

我們把能夠直接回傳的值，定義為立即數。

//emlist{
def immediate_val?(exp)
  num?(exp) 
end

def num?(exp)
  exp.is_a?(Numeric)
end
//}

函數套用，是將參數的直（會成為 Ruby 的值）套用在（Ruby 的）函數中。@<tt>{fun_val.call(*args)}是以參數@<tt>{fun_val}呼叫 Ruby 函數@<tt>{fun_val}。@<tt>{*}可以處理不定長度引數（variable-length argument），如果@<tt>{args}是@<tt>{[1,2]}，就會展開為@<tt>{fun_val.call(1,2)} ，如果是@<tt>{[1, 2, 3]}就會展開為<tt>{fun_val.call(1, 2, 3)}。

//emlist{
def apply(fun, args)
  apply_primitive_fun(fun, args)
end

def apply_primitive_fun(fun, args)
  fun_val = fun[1]
  fun_val.call(*args)
end
//}

我們來實際看看@<tt>{[:+, 1, 2]}的求值流程吧。請參照@<img>{uschemer_ruby}。要執行的程式是 list，因此我們先對一開頭的元素@<tt>{:+}求值，取得的值是 @<i>{lambda{|x, y|  x + y\}}。接著對@<tt>{1}, @<tt>{2}求值，分別取得（Ruby 的）1, 2。把這個套用在 Ruby 中，就會得到 3。使用 puts 就能用 Ruby 程式把值印出來@<fn>{fn5}。

//image[uschemer_ruby][程式語言的世界 μSchemeR 與 evaluation value 的世界 Ruby 的關係]{
//}

//emlist{
puts _eval([:+, 1, 2])
//}

執行後會顯示 3 嗎？恭喜您。這說不定是您第一次寫程式語言的 interpreter 吧。雖然是只能做加法的程式語言，您可能覺得沒有什麼真實感，但這可是正宗的程式語言 interpreter 喔。

@<tt>{[:+, [:+, 1, 2], 3]}請自己想想看這段程式的求值流程。@<tt>{_eval}可以被遞迴呼叫，這一點很重要，您注意到了嗎？

== 總結

我們在本章學到：

 * 簡單的程式運算方法（我們將這種運算稱為「求值」）
 * 函數套用的求值方法，就是對函數和參數求值，將取得的參數值套用在取得到的函數值中
 * 程式經過求值（程式的執行結果稱為 Ruby），可以取得其他世界的值 

平常沒有多想就寫出的程式@<tt>{x = y;}，其實是先對右邊求值，再將結果存放在左邊的變數的位址。雖然您可能還沒有完全了解，但事實上，程式之所以能夠執行，都是根基於本章所教的求值等概念。是不是稍微窺見了箇中奧妙呢？

//footnote[fn1][這就是程式語言的操作語義學(operational semantics)。]

//footnote[fn2][之所以不用@<tt>{eval}而是@<tt>{_eval}，是因為 Ruby 的內建函數中已經定義了@<tt>{eval}。很介意的話，可以使用 Ruby 的 module 功能分開 namespace，將@<tt>{_eval}重新定義為@<tt>{eval}。如果您要這樣做的話，第四章中用到@<tt>{parse}時，需要把@<tt>{eval}替換成＠<tt>{Kernel::eval}。]

//footnote[fn3][在 Ruby 裡，函數中最後的 expression 求得的值，就是函數回傳的值。一般來說，在函數語言中會用這樣的寫法，而不會寫成@<tt>{return} @<b>{<値>}。本書也是採用類似的立場來表述。

//footnote[fn4][car 是 Contents of the Address part of Register、cdr 是 Contents of the Decrement part of Register、之後會出現的 cons 是從 CONStruct 來的。這些都源自開發出 Lisp 的 IBM 電腦的機械語言。]

//footnote[fn5][這次要做的 μSchemeR 之後也可以做到喔。敬請期待。]