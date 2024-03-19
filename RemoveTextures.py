import bpy

# 씬의 모든 오브젝트를 순회합니다.
for obj in bpy.context.scene.objects:
    # 오브젝트 타입이 메쉬인 경우만 처리합니다.
    if obj.type == 'MESH':
        # 해당 메쉬 오브젝트의 모든 머티리얼을 순회합니다.
        for mat in obj.data.materials:
            # 머티리얼이 노드를 사용하는 경우에만 처리합니다.
            if mat.use_nodes:
                # 노드 트리를 가져옵니다.
                nodes = mat.node_tree.nodes
                # 노드 트리에서 'TEX_IMAGE' 타입의 노드(텍스쳐 노드)를 찾아 제거합니다.
                for node in nodes:
                    if node.type == 'TEX_IMAGE':
                        nodes.remove(node)

print("작업 완료: 모든 메쉬의 머티리얼에서 텍스쳐 노드를 제거하였습니다.")
