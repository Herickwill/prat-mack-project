# Entry point da Lambda
import json
import boto3
import os

def lambda_handler(event, context):
    dynamodb = boto3.client('dynamodb')
    print("Medição recebida!")
    dynamodb.put_item(
    TableName=os.environ['DYNAMODB_TABLE'],
    Item={
        'caixa_id': {'S': str(event.get("caixa_id"))},
        'timestamp': {'S': str(event.get("timestamp"))},
        'valor_sensor': {'N': str(event.get("valor_sensor"))}
        }
    )

    if(float(event.get("valor_sensor")) > 150.0):
        # Cliente SNS
        sns = boto3.client('sns', region_name='sa-east-1')

        # ARN do tópico (retirado da imagem)
        topic_arn = 'arn:aws:sns:sa-east-1:977099030484:agua-alertas'

        # Mensagem
        message = f'Identificamos uma situação crítica em relação à água na sua área. Níveis alarmantes de poluentes foram detectados na  {event.get("caixa_id")}. É crucial que você verifique e tome medidas imediatas para garantir a segurança da água que você e sua família consomem.'

        # Publicar mensagem
        response = sns.publish(
            TopicArn=topic_arn,
            Message=message,
            Subject='Problema urgente na qualidade da água!'
        )

    return {
        'statusCode': 200,
        'body': json.dumps(event['caixa_id'])
    }

