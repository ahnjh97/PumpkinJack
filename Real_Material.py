import bpy
import os

# .dds 이미지 파일들이 위치한 폴더의 경로 지정
image_folder_path = r"C:\Users\Chelsea0427\Desktop\PumpkinJack\Resources\Textures\CommonTextures\\"

for mat in bpy.data.materials:
    if not mat.use_nodes:
        mat.use_nodes = True
    nodes = mat.node_tree.nodes

    # 'Principled BSDF' 노드 찾기
    bsdf_node = next(node for node in nodes if node.type == 'BSDF_PRINCIPLED')
    
    # 머티리얼 이름에서 '.'을 기준으로 문자열을 분리하고 첫 번째 부분을 사용
    mat_base_name = mat.name.split('.')[0]
    
    # 분리된 이름과 동일한 이름의 .dds 파일 찾기
    image_path = os.path.join(image_folder_path, mat_base_name + '.dds')
    
    if os.path.exists(image_path):
        # 이미지 텍스처 노드 생성
        tex_image_node = nodes.new(type='ShaderNodeTexImage')
        tex_image_node.image = bpy.data.images.load(image_path)
        
        # 'Base Color' 입력에 이미지 텍스처 연결
        mat.node_tree.links.new(bsdf_node.inputs['Base Color'], tex_image_node.outputs['Color'])
        
        # 성공 메시지 출력
        print(f"{mat_base_name}.dds 이미지 텍스처가 {mat.name} 머티리얼에 성공적으로 할당되었습니다.")
    else:
        print(f"{mat_base_name}.dds 파일을 찾을 수 없습니다.")