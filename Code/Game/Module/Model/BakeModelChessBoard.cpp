#include "BakeModelChessBoard.hpp"

#include "Engine/Core/VertexUtils.hpp"

BakeModelChessBoard::BakeModelChessBoard()
{
    name = "ChessBoard";
}

BakeModelChessBoard::~BakeModelChessBoard()
{
}

void BakeModelChessBoard::Build()
{
    BakedModel::Build();
    std::vector<Vertex_PCUTBN>& verts   = m_vertices[0];
    std::vector<unsigned int>&  indices = m_indices[0];

    constexpr int   boardSize    = 8;
    constexpr float tileSize     = 1.0f;
    constexpr float marginSize   = tileSize / 3.0f;
    constexpr float boardZTop    = 0.0f;
    constexpr float boardZBottom = -marginSize;
    AABB2           uv           = AABB2::ZERO_TO_ONE;

    const Rgba8 lightColor(200, 200, 200);
    const Rgba8 darkColor(50, 50, 50);
    const Rgba8 borderColor(100, 60, 30);

    // Checkerboard
    for (int y = 0; y < boardSize; ++y)
    {
        for (int x = 0; x < boardSize; ++x)
        {
            Vec3  min(static_cast<float>(x), static_cast<float>(y), boardZBottom);
            Vec3  max(static_cast<float>(x) + 1, static_cast<float>(y) + 1, boardZTop);
            AABB3 square(min, max);
            bool  isLight = ((x + y) % 2 == 0);
            Rgba8 color   = isLight ? darkColor : lightColor;
            square.BuildVertices(verts, indices, color, uv);
        }
    }

    // Four-side frame housing
    // Top Bottom
    for (int i = 0; i < boardSize; ++i)
    {
        // bottom
        AABB3 b(Vec3(static_cast<float>(i), -marginSize, boardZBottom), Vec3(static_cast<float>(i) + 1, 0.f, boardZTop));
        b.BuildVertices(verts, indices, borderColor, uv);

        // top
        AABB3 t(Vec3(static_cast<float>(i), boardSize, boardZBottom), Vec3(static_cast<float>(i) + 1, boardSize + marginSize, boardZTop));
        t.BuildVertices(verts, indices, borderColor, uv);
    }

    // Left Right
    for (int j = 0; j < boardSize; ++j)
    {
        // left
        AABB3 l(Vec3(-marginSize, static_cast<float>(j), boardZBottom), Vec3(0.f, static_cast<float>(j) + 1, boardZTop));
        l.BuildVertices(verts, indices, borderColor, uv);

        // right
        AABB3 r(Vec3(boardSize, static_cast<float>(j), boardZBottom), Vec3(boardSize + marginSize, static_cast<float>(j) + 1, boardZTop));
        r.BuildVertices(verts, indices, borderColor, uv);
    }

    // Corner patch (repair gap)
    {
        AABB3 bl(Vec3(-marginSize, -marginSize, boardZBottom), Vec3(0, 0, boardZTop));
        AABB3 br(Vec3(boardSize, -marginSize, boardZBottom), Vec3(boardSize + marginSize, 0, boardZTop));
        AABB3 tl(Vec3(-marginSize, boardSize, boardZBottom), Vec3(0, boardSize + marginSize, boardZTop));
        AABB3 tr(Vec3(boardSize, boardSize, boardZBottom), Vec3(boardSize + marginSize, boardSize + marginSize, boardZTop));
        bl.BuildVertices(verts, indices, borderColor, uv);
        br.BuildVertices(verts, indices, borderColor, uv);
        tl.BuildVertices(verts, indices, borderColor, uv);
        tr.BuildVertices(verts, indices, borderColor, uv);
    }
}
