ODRP-V1

**给定**：

-  $k$ 个关键点，编号 $1, 2, \cdots, k$，起点编号为 $0$。
-  $dis_{i,j}$ 表示 $i$ 号点和 $j$ 号点的距离，其中 $i = 0,1,2, \cdots, k;\ j = 0,1,2,\cdots, k$；
- $n$ 个乘客，每个乘客的目的地 $dest_{i}$，其中 $i = 1,2, \cdots, n$。
- 每辆车的载客限制 $L$。

**变量**：

- $bus_{i,j}$ 表示第 $i$ 辆巴士经过第 $j$ 号点的顺序（整数），其中 $i=1,2,\cdots,n;\ j = 1,2,\cdots,k$。
  - 如果 $bus_{i,j} = 0$ 说明巴士没有经过这个点；
  - 如果 $bus_{i,j} = l$ 说明巴士经过了这个点，且是第 $l$ 个经过的；
- $c_i$ 表示第 $i$ 辆巴士的停靠站数量（整数），其中 $i=1,2,\cdots,n$。
- $p_{i,j}$ 表示第 $i$ 辆巴士是否 (1/0) 载了第 $j$ 个乘客（整数），其中 $i=1,2,\cdots,n;\ j = 1,2,\cdots,n$。
- $b_{i,j}$ 表示第 $i$ 辆巴士经过的第 $j$ 个点是哪一个点，其中 $i=1,2,\cdots,n;\ j = 1,2,\cdots,k$。
  - 如果 $b_{i,j} = 0$ 说明巴士线路长度没有达到 $j$；
  - 如果 $b_{i,j} = l$ 说明巴士经过的第 $j$ 个点是 $l$ 号点。

**限制**

0. 基本限制

- $$
  0 \leq bus_{i,j} \leq k, \quad i=1,2,\cdots,n;\ j = 1,2,\cdots,k
  $$

- $$
  p_{i,j}\textrm{ binary}, \quad i=1,2,\cdots,n;\ j = 1,2,\cdots,n
  $$

- $$
  0 \leq b_{i,j} \leq k, \quad i=1,2,\cdots,n;\ j = 1,2,\cdots,k
  $$

1. 巴士路线限制

- $$
  c_i = \sum_{j=1}^k [bus_{i,j} \ne 0], \quad i =1,2,\cdots,n
  $$

- $$
  [l \leq c_i] = \sum_{j=1}^k [bus_{i,j} = l], \quad i=1,2,\cdots,n;\ l=1,2,\cdots,k
  $$

- $$
  \neg [bus_{i,j}=l]\or[b_{i,l}=j], \quad i=1,2,\cdots,n;\ j = 1,2,\cdots, k;\ l=1,2,\cdots,k
  $$

- $$
  \min(c_i,l) = \sum_{j=1}^l [b_{i,j} \ne 0], \quad i=1,2,\cdots,n;\ l=1,2,\cdots,k
  $$

2. 载客限制

- $$
  \sum_{j=1}^n p_{i,j} \leq L, \quad i=1,2,\cdots,n
  $$

- $$
  \neg [p_{i,j} = 1] \lor [bus_{i, dest_j} \ne 0] = 1
  $$

- $$
  \sum_{i=1}^n p_{i,j} = 1, \quad j = 1,2,\cdots,n
  $$

**成本**

- 固定成本 $M_1 = c_r\times m$；

- 油耗成本 
  $$
  M_2 = c_b \sum_{i=1}^n \left(dis_{b_{i,k},0} + dis_{0,b_{i,1}}+\sum_{j=1}^{k-1} dis_{b_{i,j}, b_{i,j+1}}\right)
  $$

**收益**：$W$

？？？

**利润**：
$$
O = W - M_1 - M_2
$$
