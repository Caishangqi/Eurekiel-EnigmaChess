// promotion
ChessMove from=a2 to=a4 teleport=true
ChessMove from=a7 to=a2 teleport=true
ChessMove from=a4 to=a5
ChessMove from=a2 to=b1 promoteTo=queen
// Enhanced Pawn Movement
ChessMove from=a2 to=a4
ChessMove from=b7 to=b6
ChessMove from=a4 to=a5
// En Passant
ChessMove from=f2 to=f5 teleport=true
ChessMove from=g7 to=g5
ChessMove from=f5 to=g6
// Castling
ChessMove from=f1 to=f5 teleport=true
ChessMove from=a7 to=a6
ChessMove from=g1 to=f3
ChessMove from=a6 to=a5
ChessMove from=e1 to=g1
/// Direct Capture with cheat
ChessMove from=a2 to=e8 teleport=true