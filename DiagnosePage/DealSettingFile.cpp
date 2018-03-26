#include "DealSettingFile.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

DealSettingFile::DealSettingFile()
{
	item_vector.append("ReadDataStream");
	item_vector.append("CommunicationControl");
	item_vector.append("SpecialFunction");
}

DealSettingFile::~DealSettingFile()
{
}

QVector<LoaderData> DealSettingFile::AnalyseDiagnoseSettingFile(QString item_name)//������¼�����ļ�
{
	QVector<LoaderData> temp_settingfile_data_vector;
	QFile file(settingfile_path);//�򿪲���ȡ�����ļ�
	QString file_content;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_content = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("�������ļ�ʧ�ܣ�", 5);
		return temp_settingfile_data_vector;
	}
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_content.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			int temp_index = item_vector.indexOf(item_name);
			if (temp_index != -1)
			{
				QJsonObject obj = parse_doucment.object();
				if (obj.contains(item_name))
				{
					QJsonValue value1 = obj.take(item_name);
					if (value1.isArray())
					{
						QJsonArray array1 = value1.toArray();
						for (int i = 0; i < array1.size(); i++)
						{
							LoaderData temp_command_data;
							QJsonValue value2 = array1.at(i);
							QJsonObject object1 = value2.toObject();
							if (object1.contains("Caption_ID"))
							{
								QJsonValue value6 = object1.take("Caption_ID");//�������
								if (value6.isString())
								{
									QString temp_caption = GetCaption(file_content, value6.toString());
									if (temp_caption == "")
									{
										TableWrite("��ȡ " + value6.toString() + "��Ӧ��Captionʧ�ܣ�", 5);
										break;
									}
									else temp_command_data.Caption_content = temp_caption;
								}
								else
								{
									TableWrite("Caption_IDֵ�����ַ�����", 5);
									break;
								}
							}
							else
							{
								TableWrite("�ļ�������Caption_ID���ԣ�", 5);
								break;
							}
							if (object1.contains("SendCommand_ID"))
							{
								QJsonValue value4 = object1.take("SendCommand_ID");
								if (value4.isString())
								{
									SendData temp_send_data = GetSendData(file_content, value4.toString());//��÷��Ͳ���
									temp_command_data.send_data = temp_send_data;
								}
								else
								{
									TableWrite("SendCommand_IDֵ�����ַ�����", 5);
									break;
								}
							}
							else
							{
								TableWrite("�ļ�������SendCommand_ID���ԣ�", 5);
								break;
							}
							if (object1.contains("ReceiveCommand_ID"))
							{
								QJsonValue value5 = object1.take("ReceiveCommand_ID");
								if (value5.isString())
								{
									ReceiveDiagloseData temp_receive_data = GetReceiveData(file_content, value5.toString());//��ý��ܲ���
									temp_command_data.receive_data = temp_receive_data;
								}
								else
								{
									TableWrite("ReceiveCommand_IDֵ�����ַ�����", 5);
									break;
								}
							}
							else
							{
								TableWrite("�ļ�������ReceiveCommand_ID���ԣ�", 5);
								break;
							}
							temp_settingfile_data_vector.push_back(temp_command_data);
						}
					}
					else
					{
						TableWrite(item_name + "ֵ�������飡", 5);
						return temp_settingfile_data_vector;
					}
				}
				else
				{
					TableWrite("�ļ�������" + item_name + "���ԣ�", 5);
					return temp_settingfile_data_vector;
				}
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return temp_settingfile_data_vector;
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return temp_settingfile_data_vector;
	}
	return temp_settingfile_data_vector;
}

QVector<ECUInfoData> DealSettingFile::GetReadECUData()//������¼�����ļ�
{
	QVector<ECUInfoData> temp_settingfile_data_vector;
	QFile file(settingfile_path);//�򿪲���ȡ�����ļ�
	QString file_content;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_content = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("�������ļ�ʧ�ܣ�", 5);
		return temp_settingfile_data_vector;
	}
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_content.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();
			if (obj.contains("ReadECUInf"))
			{
				QJsonValue value1 = obj.take("ReadECUInf");
				if (value1.isArray())
				{
					QJsonArray array1 = value1.toArray();
					for (int i = 0; i < array1.size(); i++)
					{
						ECUInfoData temp_ecuinfo;
						QJsonValue value2 = array1.at(i);
						QJsonObject object1 = value2.toObject();
						if (object1.contains("SendCommand_ID"))
						{
							QJsonValue value4 = object1.take("SendCommand_ID");
							if (value4.isString())
							{
								SendData temp_send_data = GetSendData(file_content, value4.toString());//��÷��Ͳ���
								temp_ecuinfo.send_data = temp_send_data;
							}
							else
							{
								TableWrite("SendCommand_IDֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������SendCommand_ID���ԣ�", 5);
							break;
						}
						if (object1.contains("ReceiveCommand_ID"))
						{
							QJsonValue value5 = object1.take("ReceiveCommand_ID");
							if (value5.isString())
							{
								ReceiveDiagloseData temp_receive_data = GetReceiveData(file_content, value5.toString());//��ý��ܲ���
								temp_ecuinfo.receive_data = temp_receive_data;
							}
							else
							{
								TableWrite("ReceiveCommand_IDֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������ReceiveCommand_ID���ԣ�", 5);
							break;
						}
						if (object1.contains("WriteECU_ID"))
						{
							QJsonValue value4 = object1.take("WriteECU_ID");
							if (value4.isString())
							{
								temp_ecuinfo.writeECU_ID = value4.toString();
							}
							else
							{
								TableWrite("WriteECU_IDֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������WriteECU_ID���ԣ�", 5);
							break;
						}
						temp_settingfile_data_vector.push_back(temp_ecuinfo);
					}
				}
				else
				{
					TableWrite("ReadECUInfֵ�������飡", 5);
					return temp_settingfile_data_vector;
				}
			}
			else
			{
				TableWrite("�ļ�������ReadECUInf���ԣ�", 5);
				return temp_settingfile_data_vector;
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return temp_settingfile_data_vector;
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return temp_settingfile_data_vector;
	}
	return temp_settingfile_data_vector;
}

QVector<WriteECUData> DealSettingFile::GetWriteECUData()//������¼�����ļ�
{
	QVector<WriteECUData> temp_settingfile_data_vector;
	QFile file(settingfile_path);//�򿪲���ȡ�����ļ�
	QString file_content;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_content = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("�������ļ�ʧ�ܣ�", 5);
		return temp_settingfile_data_vector;
	}
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_content.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();
			if (obj.contains("WriteECUInf"))
			{
				QJsonValue value1 = obj.take("WriteECUInf");
				if (value1.isArray())
				{
					QJsonArray array1 = value1.toArray();
					for (int i = 0; i < array1.size(); i++)
					{
						WriteECUData temp_writeecuinfo;
						QJsonValue value2 = array1.at(i);
						QJsonObject object1 = value2.toObject();
						if (object1.contains("Caption_ID"))
						{
							QJsonValue value6 = object1.take("Caption_ID");//�������
							if (value6.isString())
							{
								QString temp_caption = GetCaption(file_content, value6.toString());
								if (temp_caption == "")
								{
									TableWrite("��ȡ " + value6.toString() + "��Ӧ��Captionʧ�ܣ�", 5);
									break;
								}
								else temp_writeecuinfo.Caption = temp_caption;
							}
							else
							{
								TableWrite("Caption_IDֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������Caption_ID���ԣ�", 5);
							break;
						}
						if (object1.contains("SendCommand_ID"))
						{
							QJsonValue value4 = object1.take("SendCommand_ID");
							if (value4.isString())
							{
								SendData temp_send_data = GetSendData(file_content, value4.toString());//��÷��Ͳ���
								temp_writeecuinfo.send_data = temp_send_data;
							}
							else
							{
								TableWrite("SendCommand_IDֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������SendCommand_ID���ԣ�", 5);
							break;
						}
						if (object1.contains("ReceiveCommand_ID"))
						{
							QJsonValue value5 = object1.take("ReceiveCommand_ID");
							if (value5.isString())
							{
								ReceiveDiagloseData temp_receive_data = GetReceiveData(file_content, value5.toString());//��ý��ܲ���
								temp_writeecuinfo.receive_data = temp_receive_data;
							}
							else
							{
								TableWrite("ReceiveCommand_IDֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������ReceiveCommand_ID���ԣ�", 5);
							break;
						}
						if (object1.contains("WriteECU_ID"))
						{
							QJsonValue value4 = object1.take("WriteECU_ID");
							if (value4.isString())
							{
								temp_writeecuinfo.writeECU_ID = value4.toString();
							}
							else
							{
								TableWrite("WriteECU_IDֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������WriteECU_ID���ԣ�", 5);
							break;
						}
						temp_settingfile_data_vector.push_back(temp_writeecuinfo);
					}
				}
				else
				{
					TableWrite("ReadECUInfֵ�������飡", 5);
					return temp_settingfile_data_vector;
				}
			}
			else
			{
				TableWrite("�ļ�������ReadECUInf���ԣ�", 5);
				return temp_settingfile_data_vector;
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return temp_settingfile_data_vector;
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return temp_settingfile_data_vector;
	}
	return temp_settingfile_data_vector;
}


QString DealSettingFile::GetCaption(QString file_data, QString caption_id)//��ö�ӦCaption�����ƣ�
{
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_data.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();

			if (obj.contains("String_Show"))
			{
				QJsonValue value1 = obj.take("String_Show");
				if (value1.isArray())
				{
					QJsonArray array1 = value1.toArray();
					for (int i = 0; i < array1.size(); i++)
					{
						QJsonValue value3 = array1.at(i);
						QJsonObject object2 = value3.toObject();
						if (object2.contains("Caption_ID"))
						{
							QJsonValue value4 = object2.take("Caption_ID");
							if (value4.isString())
							{
								if (value4.toString() == caption_id)//����caption_id
								{
									if (object2.contains("Caption"))
									{
										QJsonValue value5 = object2.take("Caption");
										if (value5.isString())
										{
											return value5.toString();//��ȡ����
										}
										else
										{
											TableWrite("Captionֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������Caption���ԣ�", 5);
										break;
									}
									break;
								}
							}
							else
							{
								TableWrite("Caption_IDֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������Caption_ID���ԣ�", 5);
							break;
						}
					}
				}
				else
				{
					TableWrite("�ļ�������String_Show���ԣ�", 5);
					return "";
				}
			}
			else
			{
				TableWrite("String_Showֵ�������飡", 5);
				return "";
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return "";
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return "";
	}
	return "";
}

SendData DealSettingFile::GetSendData(QString file_data, QString send_id)//��ö�Ӧ���Ͳ���
{
	SendData temp_send_data;
	if (send_id == "")return temp_send_data;
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_data.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();

			if (obj.contains("SendCommand"))
			{
				QJsonValue value1 = obj.take("SendCommand");
				if (value1.isArray())
				{
					QJsonArray array1 = value1.toArray();
					for (int i = 0; i < array1.size(); i++)
					{
						QJsonValue value3 = array1.at(i);
						QJsonObject object2 = value3.toObject();
						if (object2.contains("SendCommand_ID"))
						{
							QJsonValue value4 = object2.take("SendCommand_ID");
							if (value4.isString())
							{
								if (value4.toString() == send_id)//����send_id
								{
									if (object2.contains("Frame_ID"))
									{
										QJsonValue value5 = object2.take("Frame_ID");//֡ID
										if (value5.isString())
										{
											temp_send_data.frame_id = Global::changeHexToUINT(value5.toString());
										}
										else
										{
											TableWrite("Frame_IDֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������Frame_ID���ԣ�", 5);
										break;
									}

									if (object2.contains("Length"))//�ֽڳ���
									{
										QJsonValue value6 = object2.take("Length");
										if (value6.isString())
										{
											int temp_length = value6.toString().toInt();
											if (temp_length > 0)temp_send_data.byte_length = temp_length;
											else
											{
												temp_send_data.single_muti_frame_flag = 1;
											}
										}
										else
										{
											TableWrite("Lengthֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������Length���ԣ�", 5);
										break;
									}

									if (object2.contains("Data"))//data
									{
										QJsonValue value7 = object2.take("Data");
										if (value7.isString())
										{
											int pos = value7.toString().indexOf("|");
											if (pos < 0)
											{
												temp_send_data.hex_data = Global::changeHexToByteVector(value7.toString());
											}
											else
											{
												temp_send_data.data_content = value7.toString();
												temp_send_data.data_analyse_flag = true;
											}
											return temp_send_data;
										}
										else
										{
											TableWrite("Dataֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������Data���ԣ�", 5);
										break;
									}
									break;
								}
							}
							else
							{
								TableWrite("SendCommand_IDֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������SendCommand_ID���ԣ�", 5);
							break;
						}
					}
				}
				else
				{
					TableWrite("�ļ�������SendCommand���ԣ�", 5);
					return temp_send_data;
				}
			}
			else
			{
				TableWrite("SendCommandֵ�������飡", 5);
				return temp_send_data;
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return temp_send_data;
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return temp_send_data;
	}
	return temp_send_data;
}

ReceiveDiagloseData DealSettingFile::GetReceiveData(QString file_data, QString receive_id)//��ý��ܲ���
{
	ReceiveDiagloseData temp_receive_data;
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_data.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject object1 = parse_doucment.object();

			if (object1.contains("SpecialReceiveCommand"))
			{
				QJsonValue value1 = object1.take("SpecialReceiveCommand");
				if (value1.isArray())
				{
					QJsonArray array1 = value1.toArray();
					for (int i = 0; i < array1.size(); i++)
					{
						QJsonValue value2 = array1.at(i);
						QJsonObject object2 = value2.toObject();
						if (object2.contains("ReceiveCommand_ID"))
						{
							QJsonValue value3 = object2.take("ReceiveCommand_ID");
							if (value3.isString())
							{
								if (value3.toString() == receive_id)//����receive_id
								{
									if (object2.contains("Frame_ID"))
									{
										QJsonValue value4 = object2.take("Frame_ID");
										if (value4.isString())
										{
											temp_receive_data.frame_id = Global::changeHexToUINT(value4.toString());
										}
										else
										{
											TableWrite("Frame_IDֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������Frame_ID���ԣ�", 5);
										break;
									}

									if (object2.contains("Length"))//�ֽڳ���
									{
										QJsonValue value5 = object2.take("Length");
										if (value5.isString())
										{
											int temp_length = value5.toString().toInt();
											if (temp_length > 0)temp_receive_data.byte_length = temp_length;

										}
										else
										{
											TableWrite("Lengthֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������Length���ԣ�", 5);
										break;
									}
									if (object2.contains("DataConfirm"))
									{
										QJsonValue value6 = object2.take("DataConfirm");
										if (value6.isArray())
										{
											QJsonArray array2 = value6.toArray();
											for (int j = 0; j < array2.size(); j++)
											{
												LitteRecord temp_little_record;
												QJsonValue value7 = array2.at(j);
												QJsonObject object3 = value7.toObject();
												if (object3.contains("Byte_Offset"))
												{
													QJsonValue value8 = object3.take("Byte_Offset");
													if (value8.isString())
													{
														int temp_offset = value8.toString().toInt();
														if (temp_offset > 0)temp_little_record.data_offset = temp_offset;
													}
													else
													{
														TableWrite("Byte_Offsetֵ�����ַ�����", 5);
														break;
													}
												}
												else
												{
													TableWrite("�ļ�������Byte_Offset���ԣ�", 5);
													break;
												}
												if (object3.contains("Byte_Number"))
												{
													QJsonValue value9 = object3.take("Byte_Number");
													if (value9.isString())
													{
														int temp_length = value9.toString().toInt();
														if (temp_length > 0)temp_little_record.byte_length = temp_length;
													}
													else
													{
														TableWrite("Byte_Numberֵ�����ַ�����", 5);
														break;
													}
												}
												else
												{
													TableWrite("�ļ�������Byte_Number���ԣ�", 5);
													break;
												}
												if (object3.contains("Byte"))
												{
													QJsonValue value10 = object3.take("Byte");
													if (value10.isString())
													{
														int pos = value10.toString().indexOf("(");
														if (pos < 0)
														{
															temp_little_record.hex_data = Global::changeHexToByteVector(value10.toString());
														}
														else
														{
															temp_little_record.data_content = value10.toString();
															temp_little_record.data_analyse_flag = true;
														}
													}
													else
													{
														TableWrite("Byteֵ�����ַ�����", 5);
														break;
													}
												}
												else
												{
													TableWrite("�ļ�������Byte���ԣ�", 5);
													break;
												}
												temp_receive_data.litte_record_vertor.push_back(temp_little_record);
											}

										}
										else
										{
											TableWrite("DataConfirmֵ�������飡", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������DataConfirm���ԣ�", 5);
										break;
									}
									if (object2.contains("DataAnalyse"))//data������������
									{
										QJsonValue value11 = object2.take("DataAnalyse");
										if (value11.isArray())
										{
											QJsonArray array3 = value11.toArray();
											for (int j = 0; j < array3.size(); j++)
											{
												LitteRecord temp_little_record;
												QJsonValue value12 = array3.at(j);
												QJsonObject object4 = value12.toObject();
												if (object4.contains("ByteAnalyse_ID"))
												{
													QJsonValue value13 = object4.take("ByteAnalyse_ID");
													if (value13.isString())
													{
														ReceiveAnalyseData temp_data = GetByteAnalyseData(file_data, value13.toString());//���ByteAnalyse����
														temp_receive_data.data_record_vertor.push_back(temp_data);
													}
													else
													{
														TableWrite("ByteAnalyse_IDֵ�����ַ�����", 5);
														break;
													}
												}
												else
												{
													TableWrite("�ļ�������ByteAnalyse_ID���ԣ�", 5);
													break;
												}
											}
										}
										else
										{
											TableWrite("DataAnalyseֵ�������飡", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������DataAnalyse���ԣ�", 5);
										break;
									}
								}
							}
							else
							{
								TableWrite("ReceiveCommand_IDֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������ReceiveCommand_ID���ԣ�", 5);
							break;
						}
					}
				}
				else
				{
					TableWrite("�ļ�������SpecialReceiveCommand���ԣ�", 5);
					return temp_receive_data;
				}
			}
			else
			{
				TableWrite("SpecialReceiveCommandֵ�������飡", 5);
				return temp_receive_data;
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return temp_receive_data;
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return temp_receive_data;
	}
	return temp_receive_data;
}

ReceiveAnalyseData DealSettingFile::GetByteAnalyseData(QString file_data, QString ByteAnalyse_id)//����ByteAnalyse����
{
	ReceiveAnalyseData temp_receiveanalyse_data;
	if (ByteAnalyse_id == "")return temp_receiveanalyse_data;
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_data.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();

			if (obj.contains("ByteAnalyse"))
			{
				QJsonValue value1 = obj.take("ByteAnalyse");
				if (value1.isArray())
				{
					QJsonArray array1 = value1.toArray();
					for (int i = 0; i < array1.size(); i++)
					{
						QJsonValue value3 = array1.at(i);
						QJsonObject object2 = value3.toObject();
						if (object2.contains("ByteAnalyse_ID"))
						{
							QJsonValue value4 = object2.take("ByteAnalyse_ID");
							if (value4.isString())
							{
								if (value4.toString() == ByteAnalyse_id)//����ָ����ByteAnalyse_id
								{
									if (object2.contains("Caption_ID"))//����Caption��������
									{
										QJsonValue value5 = object2.take("Caption_ID");
										if (value5.isString())
										{
											QString temp_caption = GetCaption(file_data,value5.toString());
											if (temp_caption != "")
											{
												temp_receiveanalyse_data.caption_content = temp_caption;
											}
										}
										else
										{
											TableWrite("Caption_IDֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������Caption_ID���ԣ�", 5);
										break;
									}
									if (object2.contains("Caculate_Type"))
									{
										QJsonValue value6 = object2.take("Caculate_Type");
										if (value6.isString())
										{
											if (value6.toString() == "State")temp_receiveanalyse_data.Caculate_Type = 0;//״̬��0��ʾ
											else if (value6.toString() == "Numerical")temp_receiveanalyse_data.Caculate_Type = 1;//������1��ʾ
											else if (value6.toString() == "ASCII")temp_receiveanalyse_data.Caculate_Type = 2;//ASC2��2��ʾ
											else if (value6.toString() == "HEX")temp_receiveanalyse_data.Caculate_Type = 3;//HEX��3��ʾ
											else temp_receiveanalyse_data.Caculate_Type = -1;
										}
										else
										{
											TableWrite("Caculate_Typeֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������Caculate_Type���ԣ�", 5);
										break;
									}
									if (object2.contains("Unit"))//��λ
									{
										QJsonValue value7 = object2.take("Unit");
										if (value7.isString())
										{
											temp_receiveanalyse_data.Unit = value7.toString();
										}
										else
										{
											TableWrite("Unitֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������Unit���ԣ�", 5);
										break;
									}
									if (object2.contains("StartBit"))//��ʼλ��ע��������bit������byte
									{
										QJsonValue value8 = object2.take("StartBit");
										if (value8.isString())
										{
											temp_receiveanalyse_data.startbit = value8.toString().toInt();
										}
										else
										{
											TableWrite("StartBitֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������StartBit���ԣ�", 5);
										break;
									}
									if (object2.contains("BitLength"))//λ���ȣ�ע����bit
									{
										QJsonValue value9 = object2.take("BitLength");
										if (value9.isString())
										{
											temp_receiveanalyse_data.bitlength = value9.toString().toInt();
										}
										else
										{
											TableWrite("BitLengthֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������BitLength���ԣ�", 5);
										break;
									}
									if (object2.contains("InvalidValue"))//λ���ȣ�ע����bit
									{
										QJsonValue value13 = object2.take("InvalidValue");
										if (value13.isString())
										{
											QString temp_data = value13.toString();
											if (temp_data != "")temp_receiveanalyse_data.invalid_value = value13.toString().toInt();
										}
										else
										{
											TableWrite("InvalidValueֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������InvalidValue���ԣ�", 5);
										break;
									}
									if (object2.contains("Formula"))//���㹫ʽ
									{
										QJsonValue value10 = object2.take("Formula");
										if (value10.isString())
										{
											temp_receiveanalyse_data.Formula = value10.toString();
										}
										else
										{
											TableWrite("Formulaֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������Formula���ԣ�", 5);
										break;
									}
									if (object2.contains("ContentID"))//ֵ���ƣ�һ�����state״̬���ź�
									{
										QJsonValue value11 = object2.take("ContentID");
										if (value11.isString())
										{
											QMap<int, QString> temp_map = GetContent(file_data, value11.toString());
											if (temp_map.size() > 0)
											{
												temp_receiveanalyse_data.value_content_map = temp_map;
											}
										}
										else
										{
											TableWrite("ContentIDֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������ContentID���ԣ�", 5);
										break;
									}
									if (object2.contains("Format"))//��ʽ���ر�Ҫע��date��ʽ����Ҫ�������������һ��
									{
										QJsonValue value12 = object2.take("Format");
										if (value12.isString())
										{
											temp_receiveanalyse_data.Format = value12.toString();
										}
										else
										{
											TableWrite("Formatֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������Format���ԣ�", 5);
										break;
									}
									if (object2.contains("StoreName"))//λ���ȣ�ע����bit
									{
										QJsonValue value14 = object2.take("StoreName");
										if (value14.isString())
										{
											temp_receiveanalyse_data.store_name = value14.toString();
										}
										else
										{
											TableWrite("StoreNameֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������StoreName���ԣ�", 5);
										break;
									}
									break;
								}
							}
							else
							{
								TableWrite("SendCommand_IDֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������SendCommand_ID���ԣ�", 5);
							break;
						}
					}
				}
				else
				{
					TableWrite("ByteAnalyseֵ�������飡", 5);
					return temp_receiveanalyse_data;
				}
			}
			else
			{
				TableWrite("�ļ�������ByteAnalyse���ԣ�", 5);
				return temp_receiveanalyse_data;
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return temp_receiveanalyse_data;
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return temp_receiveanalyse_data;
	}
	return temp_receiveanalyse_data;
}

QMap<int, QString> DealSettingFile::GetContent(QString file_data, QString content_id)//��ö�ӦCaptionID�������е�ֵ-���ƶ�
{
	QMap<int, QString> temp_content_map;
	if (content_id == "")return temp_content_map;
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_data.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();
			if (obj.contains("Content"))
			{
				QJsonValue value1 = obj.take("Content");
				QJsonObject object1 = value1.toObject();
				if (object1.contains(content_id))//����content_id�����state״̬���źţ�֮�������ֵ-���ƶ���ȡ����
				{
					QJsonValue value2 = object1.take(content_id);
					if (value2.isArray())
					{
						QJsonArray array1 = value2.toArray();
						for (int i = 0; i < array1.size(); i++)
						{
							int temp_key = -1;
							QString temp_value;
							QJsonValue value3 = array1.at(i);
							QJsonObject object2 = value3.toObject();
							if (object2.contains("Signal_Value"))
							{
								QJsonValue value5 = object2.take("Signal_Value");//ֵ
								if (value5.isString())
								{
									temp_key = value5.toString().toInt();
								}
								else
								{
									TableWrite("Signal_Valueֵ�����ַ�����", 5);
									break;
								}
							}
							else
							{
								TableWrite("�ļ�������Signal_Value���ԣ�", 5);
								break;
							}
							if (object2.contains("Show"))
							{
								QJsonValue value6 = object2.take("Show");//����
								if (value6.isString())
								{
									temp_value = value6.toString();
								}
								else
								{
									TableWrite("Signal_Valueֵ�����ַ�����", 5);
									break;
								}
							}
							else
							{
								TableWrite("�ļ�������Signal_Value���ԣ�", 5);
								break;
							}
							if (temp_key != -1 && temp_value != "")
							{
								temp_content_map.insert(temp_key, temp_value);
							}
						}
					}
					else
					{
						TableWrite(content_id + "ֵ�������飡", 5);
						return temp_content_map;
					}
				}
				else
				{
					TableWrite("�ļ�������" + content_id + "���ԣ�", 5);
					return temp_content_map;
				}
			}
			else
			{
				TableWrite("�ļ�������Content���ԣ�", 5);
				return temp_content_map;
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return temp_content_map;
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return temp_content_map;
	}
	return temp_content_map;
}

QVector<ControlData> DealSettingFile::GetOperateControlData()//������¼�����ļ�
{
	QVector<ControlData> temp_settingfile_data_vector;
	QFile file(settingfile_path);//�򿪲���ȡ�����ļ�
	QString file_content;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_content = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("�������ļ�ʧ�ܣ�", 5);
		return temp_settingfile_data_vector;
	}
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_content.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();
			if (obj.contains("OperateControl"))
			{
				QJsonValue value1 = obj.take("OperateControl");
				if (value1.isArray())
				{
					QJsonArray array1 = value1.toArray();
					for (int i = 0; i < array1.size(); i++)
					{
						ControlData temp_control_data;
						QJsonValue value2 = array1.at(i);
						QJsonObject object1 = value2.toObject();
						if (object1.contains("Caption_ID"))
						{
							QJsonValue value3 = object1.take("Caption_ID");//�������
							if (value3.isString())
							{
								QString temp_caption = GetCaption(file_content, value3.toString());
								if (temp_caption == "")
								{
									TableWrite("��ȡ " + value3.toString() + "��Ӧ��Captionʧ�ܣ�", 5);
									break;
								}
								else temp_control_data.item_caption = temp_caption;
							}
							else
							{
								TableWrite("Caption_IDֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������Caption_ID���ԣ�", 5);
							break;
						}
						if (object1.contains("ControlItem"))
						{
							QJsonValue value4 = object1.take("ControlItem");
							if (value4.isArray())
							{
								QJsonArray array2 = value4.toArray();
								for (int j = 0; j < array2.size(); j++)
								{
									LoaderData temp_command_data;
									QJsonValue value5 = array2.at(j);
									QJsonObject object2 = value5.toObject();
									if (object2.contains("Caption_ID"))
									{
										QJsonValue value6 = object2.take("Caption_ID");
										if (value6.isString())
										{
											QString temp_caption = GetCaption(file_content, value6.toString());
											if (temp_caption == "")
											{
												TableWrite("��ȡ " + value6.toString() + "��Ӧ��Captionʧ�ܣ�", 5);
												break;
											}
											else
											{
												temp_command_data.Caption_content = temp_caption;
											}
										}
										else
										{
											TableWrite("Caption_IDֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������Caption_ID���ԣ�", 5);
										break;
									}
									if (object2.contains("SendCommand_ID"))
									{
										QJsonValue value7 = object2.take("SendCommand_ID");
										if (value7.isString())
										{
											SendData temp_send_data = GetSendData(file_content, value7.toString());//��÷��Ͳ���
											temp_command_data.send_data = temp_send_data;
										}
										else
										{
											TableWrite("SendCommand_IDֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������SendCommand_ID���ԣ�", 5);
										break;
									}
									if (object2.contains("ReceiveCommand_ID"))
									{
										QJsonValue value8 = object2.take("ReceiveCommand_ID");
										if (value8.isString())
										{
											ReceiveDiagloseData temp_receive_data = GetReceiveData(file_content, value8.toString());//��ý��ܲ���
											temp_command_data.receive_data = temp_receive_data;
										}
										else
										{
											TableWrite("ReceiveCommand_IDֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������ReceiveCommand_ID���ԣ�", 5);
										break;
									}
									temp_control_data.command_vector.append(temp_command_data);
								}
							}
							else
							{
								TableWrite("ControlItemֵ�������飡", 5);
								return temp_settingfile_data_vector;
							}
						}
						temp_settingfile_data_vector.append(temp_control_data);
					}					
				}
				else
				{
					TableWrite("OperateControlֵ�������飡", 5);
					return temp_settingfile_data_vector;
				}
			}
			else
			{
				TableWrite("�ļ�������OperateControl���ԣ�", 5);
				return temp_settingfile_data_vector;
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return temp_settingfile_data_vector;
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return temp_settingfile_data_vector;
	}
	return temp_settingfile_data_vector;
}

QVector<ControlData> DealSettingFile::GetRoutineControlData()//������¼�����ļ�
{
	QVector<ControlData> temp_settingfile_data_vector;
	QFile file(settingfile_path);//�򿪲���ȡ�����ļ�
	QString file_content;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_content = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("�������ļ�ʧ�ܣ�", 5);
		return temp_settingfile_data_vector;
	}
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_content.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();
			if (obj.contains("RoutineControl"))
			{
				QJsonValue value1 = obj.take("RoutineControl");
				if (value1.isArray())
				{
					QJsonArray array1 = value1.toArray();
					for (int i = 0; i < array1.size(); i++)
					{
						ControlData temp_control_data;
						QJsonValue value2 = array1.at(i);
						QJsonObject object1 = value2.toObject();
						if (object1.contains("Control_ID"))
						{
							QJsonValue value10 = object1.take("Control_ID");//�������
							if (value10.isString())
							{
								QString temp_id = value10.toString();
								if (temp_id == "")
								{
									TableWrite("��ȡ " + value10.toString() + "��Ӧ��Control_IDʧ�ܣ�", 5);
									break;
								}
								else temp_control_data.item_id = temp_id;
							}
							else
							{
								TableWrite("Control_IDֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������Control_ID���ԣ�", 5);
							break;
						}
						if (object1.contains("Caption_ID"))
						{
							QJsonValue value3 = object1.take("Caption_ID");//�������
							if (value3.isString())
							{
								QString temp_caption = GetCaption(file_content, value3.toString());
								if (temp_caption == "")
								{
									TableWrite("��ȡ " + value3.toString() + "��Ӧ��Captionʧ�ܣ�", 5);
									break;
								}
								else temp_control_data.item_caption = temp_caption;
							}
							else
							{
								TableWrite("Caption_IDֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������Caption_ID���ԣ�", 5);
							break;
						}
						if (object1.contains("ControlItem"))
						{
							QJsonValue value4 = object1.take("ControlItem");
							if (value4.isArray())
							{
								QJsonArray array2 = value4.toArray();
								for (int j = 0; j < array2.size(); j++)
								{
									LoaderData temp_command_data;
									QJsonValue value5 = array2.at(j);
									QJsonObject object2 = value5.toObject();
									if (object2.contains("Caption_ID"))
									{
										QJsonValue value6 = object2.take("Caption_ID");
										if (value6.isString())
										{
											QString temp_caption = GetCaption(file_content, value6.toString());
											if (temp_caption == "")
											{
												TableWrite("��ȡ " + value6.toString() + "��Ӧ��Captionʧ�ܣ�", 5);
												break;
											}
											else
											{
												temp_command_data.Caption_content = temp_caption;
											}
										}
										else
										{
											TableWrite("Caption_IDֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������Caption_ID���ԣ�", 5);
										break;
									}
									if (object2.contains("SendCommand_ID"))
									{
										QJsonValue value7 = object2.take("SendCommand_ID");
										if (value7.isString())
										{
											SendData temp_send_data = GetSendData(file_content, value7.toString());//��÷��Ͳ���
											temp_command_data.send_data = temp_send_data;
										}
										else
										{
											TableWrite("SendCommand_IDֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������SendCommand_ID���ԣ�", 5);
										break;
									}
									if (object2.contains("ReceiveCommand_ID"))
									{
										QJsonValue value8 = object2.take("ReceiveCommand_ID");
										if (value8.isString())
										{
											ReceiveDiagloseData temp_receive_data = GetReceiveData(file_content, value8.toString());//��ý��ܲ���
											temp_command_data.receive_data = temp_receive_data;
										}
										else
										{
											TableWrite("ReceiveCommand_IDֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������ReceiveCommand_ID���ԣ�", 5);
										break;
									}
									temp_control_data.command_vector.append(temp_command_data);
								}
							}
							else
							{
								TableWrite("ControlItemֵ�������飡", 5);
								return temp_settingfile_data_vector;
							}
						}
						temp_settingfile_data_vector.append(temp_control_data);
					}
				}
				else
				{
					TableWrite("RoutineControlֵ�������飡", 5);
					return temp_settingfile_data_vector;
				}
			}
			else
			{
				TableWrite("�ļ�������RoutineControl���ԣ�", 5);
				return temp_settingfile_data_vector;
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return temp_settingfile_data_vector;
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return temp_settingfile_data_vector;
	}
	return temp_settingfile_data_vector;
}


void DealSettingFile::TableWrite(QString message_content, int message_type)//����������Ϣ����
{
	emit display_messageSignal(message_content, message_type);
}

QVector<ModelInfo> DealSettingFile::GetVehicleModel()//��ȡVehicleSelect�ļ������Ͳ����ļ���
{
	QVector<ModelInfo> temp_settingfile_data_vector;
	QFile file(vehiclefile_path);//�򿪲���ȡ�����ļ�
	QString file_content;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_content = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("�򿪳���ƽ̨�ļ�ʧ�ܣ�", 5);
		return temp_settingfile_data_vector;
	}
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_content.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();
			if (obj.contains("Vehicle"))
			{
				QJsonValue name_value = obj.take("Vehicle");
				if (name_value.isArray())
				{
					QJsonArray array1 = name_value.toArray();
					for (int i = 0; i < array1.size(); i++)
					{
						QJsonValue value1 = array1.at(i);
						QJsonObject object1 = value1.toObject();
						ModelInfo temp_modelinfo;
						if (object1.contains("Vehicle"))
						{
							QJsonValue value2 = object1.take("Vehicle");
							if (value2.isString())
							{
								temp_modelinfo.model_name = value2.toString();
							}
							else
							{
								TableWrite("Vehicleֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������Vehicle���ԣ�", 5);
							break;
						}
						if (object1.contains("PicturePath"))
						{
							QJsonValue value3 = object1.take("PicturePath");
							if (value3.isString())
							{
								temp_modelinfo.picture_path = value3.toString();
							}
							else
							{
								TableWrite("PicturePathֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������PicturePath���ԣ�", 5);
							break;
						}
						if (object1.contains("FilePath"))
						{
							QJsonValue value4 = object1.take("FilePath");
							if (value4.isString())
							{
								temp_modelinfo.settingfile_path = value4.toString();
							}
							else
							{
								TableWrite("FilePathֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������FilePath���ԣ�", 5);
							break;
						}
						if (object1.contains("DiagnosisBased"))
						{
							QJsonValue value5 = object1.take("DiagnosisBased");
							if (value5.isString())
							{
								temp_modelinfo.DiagnosisBased = value5.toString();
							}
							else
							{
								TableWrite("DiagnosisBasedֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������DiagnosisBased���ԣ�", 5);
							break;
						}
						if (object1.contains("BoundRate"))
						{
							QJsonValue value6 = object1.take("BoundRate");
							if (value6.isString())
							{
								temp_modelinfo.Baud_Setting = value6.toString();
							}
							else
							{
								TableWrite("BoundRateֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������BoundRate���ԣ�", 5);
							break;
						}
						temp_settingfile_data_vector.push_back(temp_modelinfo);
					}
				}
				else
				{
					TableWrite("Vehicleֵ�������飡", 5);
					return temp_settingfile_data_vector;
				}
			}
			else
			{
				TableWrite("�ļ�������Vehicle���ԣ�", 5);
				return temp_settingfile_data_vector;
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return temp_settingfile_data_vector;
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return temp_settingfile_data_vector;
	}
	return temp_settingfile_data_vector;
}

QVector<LoaderData> DealSettingFile::AnalyseDiagnoseVehicleFile(QString DiagnosisBased_content)//������¼�����ļ�
{
	QVector<LoaderData> temp_settingfile_data_vector;
	QFile file(vehiclefile_path);//�򿪲���ȡ�����ļ�
	QString file_data;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_data = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("�򿪳���ƽ̨�ļ�ʧ�ܣ�", 5);
		return temp_settingfile_data_vector;
	}
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_data.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();

			if (obj.contains("DiagnosisBased"))
			{
				QJsonValue value1 = obj.take("DiagnosisBased");
				QJsonObject object1 = value1.toObject();
				if (object1.contains(DiagnosisBased_content))
				{
					QJsonValue value2 = object1.take(DiagnosisBased_content);
					if (value2.isArray())
					{
						QJsonArray array1 = value2.toArray();
						for (int i = 0; i < array1.size(); i++)
						{
							LoaderData temp_line_data;
							QString temp_name, temp_path;
							QJsonValue value3 = array1.at(i);
							QJsonObject object2 = value3.toObject();
							if (object2.contains("Caption_ID"))
							{
								QJsonValue value6 = object2.take("Caption_ID");
								if (value6.isString())
								{
									QString temp_caption = GetCaption(file_data, value6.toString());
									if (temp_caption == "")
									{
										TableWrite("��ȡ " + value6.toString() + "��Ӧ��Captionʧ�ܣ�", 5);
										break;
									}
									temp_line_data.Caption_content = temp_caption;
								}
								else
								{
									TableWrite("Caption_IDֵ�����ַ�����", 5);
									break;
								}
							}
							else
							{
								TableWrite("�ļ�������Caption_ID���ԣ�", 5);
								break;
							}

							if (object2.contains("SendCommand_ID"))
							{
								QJsonValue value7 = object2.take("SendCommand_ID");
								if (value7.isString())
								{
									SendData temp_send_data = GetSendData(file_data, value7.toString());
									temp_line_data.send_data = temp_send_data;
								}
								else
								{
									TableWrite("SendCommand_IDֵ�����ַ�����", 5);
									break;
								}
							}
							else
							{
								TableWrite("�ļ�������SendCommand_ID���ԣ�", 5);
								break;
							}
							if (object2.contains("ReceiveCommand_ID"))
							{
								QJsonValue value12 = object2.take("ReceiveCommand_ID");
								if (value12.isString())
								{
									ReceiveDiagloseData temp_receive_data = GetReceiveData(file_data, value12.toString());
									temp_line_data.receive_data = temp_receive_data;
								}
								else
								{
									TableWrite("ReceiveCommand_IDֵ�����ַ�����", 5);
									break;
								}
							}
							else
							{
								TableWrite("�ļ�������ReceiveCommand_ID���ԣ�", 5);
								break;
							}
							if (object2.contains("Dely_Time"))
							{
								QJsonValue value9 = object2.take("Dely_Time");
								if (value9.isString())
								{
									temp_line_data.Dely_Time = value9.toString().toInt();
								}
								else
								{
									TableWrite("Dely_Timeֵ�����ַ�����", 5);
									break;
								}
							}
							else
							{
								TableWrite("�ļ�������Dely_Time���ԣ�", 5);
								break;
							}
							if (object2.contains("CircleTimes"))
							{
								QJsonValue value10 = object2.take("CircleTimes");
								if (value10.isString())
								{
									int pos = value10.toString().indexOf("(");
									if (pos < 0)
									{
										temp_line_data.circle_times = value10.toString().toInt();
									}
									else
									{
										temp_line_data.CircleTimes_content = value10.toString();
										temp_line_data.circletime_analyse_flag = true;
									}
								}
								else
								{
									TableWrite("CircleTimesֵ�����ַ�����", 5);
									break;
								}
							}
							else
							{
								TableWrite("�ļ�������CircleTimes���ԣ�", 5);
								break;
							}
							temp_settingfile_data_vector.push_back(temp_line_data);
						}
					}
					else
					{
						TableWrite("BootLoader_Stepֵ�������飡", 5);
						return temp_settingfile_data_vector;
					}
				}
			}
			else
			{
				TableWrite("�ļ�������BootLoader_Step���ԣ�", 5);
				return temp_settingfile_data_vector;
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return temp_settingfile_data_vector;
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return temp_settingfile_data_vector;
	}
	return temp_settingfile_data_vector;
}

QMap<int, QString> DealSettingFile::GetPageList()
{
	QMap<int, QString> list_item_map;
	QFile file(settingfile_path);//�򿪲���ȡ�����ļ�
	QString file_data;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_data = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("�������ļ�ʧ�ܣ�", 5);
		return list_item_map;
	}
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_data.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();
			if (obj.contains("Function"))
			{
				QJsonValue value1 = obj.take("Function");
				if (value1.isArray())
				{
					int item_id = -1;
					QJsonArray array1 = value1.toArray();
					for (int i = 0; i < array1.size(); i++)
					{
						QString item_name;
						QJsonValue value2 = array1.at(i);
						QJsonObject object1 = value2.toObject();
						if (object1.contains("Name"))
						{
							QJsonValue value5 = object1.take("Name");
							if (value5.isString())
							{
								item_name = value5.toString();
								item_id++;
							}
							else
							{
								TableWrite("Nameֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������Name���ԣ�", 5);
							break;
						}
						list_item_map.insert(item_id, item_name);
					}
				}
				else
				{
					TableWrite("Functionֵ�������飡", 5);
					return list_item_map;
				}
			}
			else
			{
				TableWrite("�ļ�������Function���ԣ�", 5);
				return list_item_map;
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return list_item_map;
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return list_item_map;
	}
	return list_item_map;
}

LoaderData DealSettingFile::GetSingleCommandFromDiagnoseSettingFile(QString function_name, QString command_caption)//������¼�����ļ�
{
	LoaderData temp_command_data;
	QFile file(settingfile_path);//�򿪲���ȡ�����ļ�
	QString file_data;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_data = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("�������ļ�ʧ�ܣ�", 5);
		return temp_command_data;
	}
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_data.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();
			if (obj.contains(function_name))
			{
				QJsonValue value1 = obj.take(function_name);
				if (value1.isArray())
				{
					QJsonArray array1 = value1.toArray();
					for (int i = 0; i < array1.size(); i++)
					{
						QJsonValue value2 = array1.at(i);
						QJsonObject object1 = value2.toObject();
						if (object1.contains("Caption_ID"))
						{
							QJsonValue value6 = object1.take("Caption_ID");//�������
							if (value6.isString())
							{
								QString temp_caption = GetCaption(file_data, value6.toString());
								if (temp_caption == "")
								{
									TableWrite("��ȡ " + value6.toString() + "��Ӧ��Captionʧ�ܣ�", 5);
									break;
								}
								else
								{
									if (temp_caption == command_caption)
									{
										temp_command_data.Caption_content = temp_caption;
										if (object1.contains("SendCommand_ID"))
										{
											QJsonValue value4 = object1.take("SendCommand_ID");
											if (value4.isString())
											{
												SendData temp_send_data = GetSendData(file_data, value4.toString());//��÷��Ͳ���
												temp_command_data.send_data = temp_send_data;
											}
											else
											{
												TableWrite("SendCommand_IDֵ�����ַ�����", 5);
												break;
											}
										}
										else
										{
											TableWrite("�ļ�������SendCommand_ID���ԣ�", 5);
											break;
										}
										if (object1.contains("ReceiveCommand_ID"))
										{
											QJsonValue value5 = object1.take("ReceiveCommand_ID");
											if (value5.isString())
											{
												//ReceiveData temp_receive_data = GetReceiveData(file_data, value5.toString());//��ý��ܲ���
												ReceiveDiagloseData temp_receive_data = GetReceiveData(file_data, value5.toString());//��ý��ܲ���
												temp_command_data.receive_data = temp_receive_data;
											}
											else
											{
												TableWrite("ReceiveCommand_IDֵ�����ַ�����", 5);
												break;
											}
										}
										else
										{
											TableWrite("�ļ�������ReceiveCommand_ID���ԣ�", 5);
											break;
										}
										break;
									}

								}
							}
							else
							{
								TableWrite("Caption_IDֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������Caption_ID���ԣ�", 5);
							break;
						}
					}
				}
				else
				{
					TableWrite(function_name + "ֵ�������飡", 5);
					return temp_command_data;
				}
			}
			else
			{
				TableWrite("�ļ�������" + function_name + "���ԣ�", 5);
				return temp_command_data;
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return temp_command_data;
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return temp_command_data;
	}
	return temp_command_data;
}


ReceiveDiagloseData DealSettingFile::AnalyseSnapshotLib(QString function_name, QString SnapshotDID)
{
	ReceiveDiagloseData temp_ReceiveDiagloseData;
	QFile file(settingfile_path);//�򿪲���ȡ�����ļ�
	QString file_content;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_content = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("�������ļ�ʧ�ܣ�", 5);
		return temp_ReceiveDiagloseData;
	}
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_content.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();
			if (obj.contains(function_name))
			{
				QJsonValue value1 = obj.take(function_name);
				if (value1.isArray())
				{
					QJsonArray array1 = value1.toArray();
					for (int j = 0; j < array1.size(); j++)
					{
						QJsonValue value8 = array1.at(j);
						QJsonObject object3 = value8.toObject();

						if (object3.contains("SnapshotDID"))
						{
							QJsonValue value2 = object3.take("SnapshotDID");
							if (value2.isString())
							{
								if (value2.toString() == SnapshotDID)
								{
									if (object3.contains("Byte_Number"))
									{
										QJsonValue value3 = object3.take("Byte_Number");
										if (value3.isString())
										{
											if (value3.toString().toInt() > 0)
											{
												temp_ReceiveDiagloseData.byte_length = value3.toString().toInt();
											}
										}
										else
										{
											TableWrite("Byte_Lengthֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("Byte_Numberֵ�����ַ�����", 5);
										break;
									}
									QVector<ReceiveAnalyseData> AnalyseData_vector;
									if (object3.contains("SnapshotDID_Data"))//data������������
									{
										QJsonValue value7 = object3.take("SnapshotDID_Data");
										if (value7.isArray())
										{
											QJsonArray array2 = value7.toArray();
											for (int j = 0; j < array2.size(); j++)
											{
												LitteRecord temp_little_record;
												QJsonValue value8 = array2.at(j);
												QJsonObject object3 = value8.toObject();
												if (object3.contains("ByteAnalyse_ID"))
												{
													QJsonValue value9 = object3.take("ByteAnalyse_ID");
													if (value9.isString())
													{
														ReceiveAnalyseData temp_data = GetByteAnalyseData(file_content, value9.toString());//���ByteAnalyse����
														AnalyseData_vector.push_back(temp_data);
													}
													else
													{
														TableWrite("ByteAnalyse_IDֵ�����ַ�����", 5);
														break;
													}
												}
												else
												{
													TableWrite("�ļ�������ByteAnalyse_ID���ԣ�", 5);
													break;
												}
											}
											temp_ReceiveDiagloseData.data_record_vertor = AnalyseData_vector;
										}
										else
										{
											TableWrite("SnapshotDID_Dataֵ�������飡", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������SnapshotDID_Data���ԣ�", 5);
										break;
									}
									break;
								}
							}
							else
							{
								TableWrite("SnapshotDIDֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������SnapshotDID���ԣ�", 5);
							break;
						}
					}
				}
				else
				{
					TableWrite(function_name + "ֵ�������飡", 5);
					return temp_ReceiveDiagloseData;
				}
			}
			else
			{
				TableWrite("�ļ�������" + function_name + "���ԣ�", 5);
				return temp_ReceiveDiagloseData;
			}

		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return temp_ReceiveDiagloseData;
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return temp_ReceiveDiagloseData;
	}
	return temp_ReceiveDiagloseData;
}

QString DealSettingFile::GetByteAnalyseID()//����ByteAnalyse����
{
	QFile file(settingfile_path);//�򿪲���ȡ�����ļ�
	QString file_content;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_content = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("�������ļ�ʧ�ܣ�", 5);
		return "";
	}
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_content.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();

			if (obj.contains("ByteAnalyseData"))
			{
				QJsonValue value1 = obj.take("ByteAnalyseData");
				QJsonObject object1 = value1.toObject();
				if (object1.contains("ByteAnalyse_ID"))
				{
					QJsonValue value2 = object1.take("ByteAnalyse_ID");
					if (value2.isString())
					{
						return value2.toString();
					}
					else
					{
						TableWrite("ByteAnalyse_IDֵ�����ַ�����", 5);
						return "";
					}
				}
				else
				{
					TableWrite("�ļ�������ByteAnalyse_ID���ԣ�", 5);
					return "";
				}

			}
			else
			{
				TableWrite("�ļ�������ByteAnalyseData���ԣ�", 5);
				return "";
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return "";
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return "";
	}
	return "";
}

FaultInfo DealSettingFile::GetFaultDetailInfo(QString temp_fault_code)
{
	FaultInfo temp_fault_data;
	if (temp_fault_code == "")return temp_fault_data;

	QFile file(settingfile_path);//�򿪲���ȡ�����ļ�
	QString file_content;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_content = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("�������ļ�ʧ�ܣ�", 5);
		return temp_fault_data;
	}
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_content.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();

			if (obj.contains("DTCLib"))
			{
				QJsonValue value1 = obj.take("DTCLib");
				if (value1.isArray())
				{
					QJsonArray array1 = value1.toArray();
					for (int i = 0; i < array1.size(); i++)
					{
						QJsonValue value3 = array1.at(i);
						QJsonObject object2 = value3.toObject();
						if (object2.contains("FaultCode"))
						{
							QJsonValue value4 = object2.take("FaultCode");
							if (value4.isString())
							{
								if (value4.toString() == temp_fault_code)//����FaultCode
								{
									if (object2.contains("DTC"))
									{
										QJsonValue value5 = object2.take("DTC");//DTC
										if (value5.isString())
										{
											temp_fault_data.dtc_code = value5.toString();
										}
										else
										{
											TableWrite("DTCֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������DTC���ԣ�", 5);
										break;
									}

									if (object2.contains("Caption_ID"))//�ֽڳ���
									{
										QJsonValue value6 = object2.take("Caption_ID");
										if (value6.isString())
										{
											temp_fault_data.fault_name = GetCaption(file_content,value6.toString());
										}
										else
										{
											TableWrite("Caption_IDֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������Caption_ID���ԣ�", 5);
										break;
									}

									if (object2.contains("FaultCauses_ID"))//data
									{
										QJsonValue value7 = object2.take("FaultCauses_ID");
										if (value7.isString())
										{
											temp_fault_data.fault_reasion = GetFaultCauses(value7.toString());
										}
										else
										{
											TableWrite("FaultCauses_IDֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������FaultCauses_ID���ԣ�", 5);
										break;
									}
									if (object2.contains("CorrecticveAction_ID"))//data
									{
										QJsonValue value8 = object2.take("CorrecticveAction_ID");
										if (value8.isString())
										{
											temp_fault_data.repair_info = GetCorrecticveAction(value8.toString());
										}
										else
										{
											TableWrite("CorrecticveAction_IDֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������CorrecticveAction_ID���ԣ�", 5);
										break;
									}
									break;
								}
							}
							else
							{
								TableWrite("FaultCodeֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������FaultCode���ԣ�", 5);
							break;
						}
					}
				}
				else
				{
					TableWrite("DTCLibֵ�������飡", 5);
					return temp_fault_data;
				}
			}
			else
			{
				TableWrite("�ļ�������DTCLib���ԣ�", 5);
				return temp_fault_data;
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return temp_fault_data;
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return temp_fault_data;
	}
	return temp_fault_data;
}

QString DealSettingFile::GetFaultCauses(QString FaultCauses_ID)//��ö�ӦFaultCauses
{
	QFile file(settingfile_path);//�򿪲���ȡ�����ļ�
	QString file_content;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_content = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("�������ļ�ʧ�ܣ�", 5);
		return "";
	}
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_content.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();

			if (obj.contains("FaultCauses"))
			{
				QJsonValue value1 = obj.take("FaultCauses");
				if (value1.isArray())
				{
					QJsonArray array1 = value1.toArray();
					for (int i = 0; i < array1.size(); i++)
					{
						QJsonValue value3 = array1.at(i);
						QJsonObject object2 = value3.toObject();
						if (object2.contains("FaultCauses_ID"))
						{
							QJsonValue value4 = object2.take("FaultCauses_ID");
							if (value4.isString())
							{
								if (value4.toString() == FaultCauses_ID)
								{
									if (object2.contains("Caption"))
									{
										QJsonValue value5 = object2.take("Caption");
										if (value5.isString())
										{
											return value5.toString();
										}
										else
										{
											TableWrite("Captionֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������Caption���ԣ�", 5);
										break;
									}
									break;
								}
							}
							else
							{
								TableWrite("FaultCauses_IDֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������FaultCauses_ID���ԣ�", 5);
							break;
						}
					}
				}
				else
				{
					TableWrite("FaultCausesֵ�������飡", 5);
					return "";
				}
			}
			else
			{
				TableWrite("�ļ�������FaultCauses���ԣ�", 5);
				return "";
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return "";
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return "";
	}
	return "";
}

QString DealSettingFile::GetCorrecticveAction(QString CorrecticveAction_ID)//��ö�ӦCorrecticveAction
{
	QFile file(settingfile_path);//�򿪲���ȡ�����ļ�
	QString file_content;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_content = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("�������ļ�ʧ�ܣ�", 5);
		return "";
	}
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_content.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();

			if (obj.contains("CorrecticveAction"))
			{
				QJsonValue value1 = obj.take("CorrecticveAction");
				if (value1.isArray())
				{
					QJsonArray array1 = value1.toArray();
					for (int i = 0; i < array1.size(); i++)
					{
						QJsonValue value3 = array1.at(i);
						QJsonObject object2 = value3.toObject();
						if (object2.contains("CorrecticveAction_ID"))
						{
							QJsonValue value4 = object2.take("CorrecticveAction_ID");
							if (value4.isString())
							{
								if (value4.toString() == CorrecticveAction_ID)
								{
									if (object2.contains("Caption"))
									{
										QJsonValue value5 = object2.take("Caption");
										if (value5.isString())
										{
											return value5.toString();
										}
										else
										{
											TableWrite("Captionֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������Caption���ԣ�", 5);
										break;
									}
									break;
								}
							}
							else
							{
								TableWrite("CorrecticveAction_IDֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������CorrecticveAction_ID���ԣ�", 5);
							break;
						}
					}
				}
				else
				{
					TableWrite("CorrecticveActionֵ�������飡", 5);
					return "";
				}
			}
			else
			{
				TableWrite("�ļ�������CorrecticveAction���ԣ�", 5);
				return "";
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return "";
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return "";
	}
	return "";
}

QString DealSettingFile::GetByteAnalyseTimeState(QString ByteAnalyse_id, BYTE analyse_byte)//����ByteAnalyse����
{
	QFile file(settingfile_path);//�򿪲���ȡ�����ļ�
	QString file_content;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		file_content = file.readAll();
		file.close();
	}
	else
	{
		TableWrite("�������ļ�ʧ�ܣ�", 5);
		return "";
	}
	if (ByteAnalyse_id == "")return "";
	int Caculate_Type = -1;
	int start_bit = -1;
	int bit_length = 0;
	QMap<int, QString> temp_content_map;
	QJsonParseError json_error;
	QJsonDocument parse_doucment = QJsonDocument::fromJson(file_content.toUtf8(), &json_error);
	if (json_error.error == QJsonParseError::NoError)
	{
		if (parse_doucment.isObject())
		{
			QJsonObject obj = parse_doucment.object();

			if (obj.contains("ByteAnalyse"))
			{
				QJsonValue value1 = obj.take("ByteAnalyse");
				if (value1.isArray())
				{
					QJsonArray array1 = value1.toArray();
					for (int i = 0; i < array1.size(); i++)
					{
						QJsonValue value3 = array1.at(i);
						QJsonObject object2 = value3.toObject();
						if (object2.contains("ByteAnalyse_ID"))
						{
							QJsonValue value4 = object2.take("ByteAnalyse_ID");
							if (value4.isString())
							{
								if (value4.toString() == ByteAnalyse_id)//����ָ����ByteAnalyse_id
								{
									if (object2.contains("Caculate_Type"))
									{
										QJsonValue value6 = object2.take("Caculate_Type");
										if (value6.isString())
										{
											if (value6.toString() == "State")Caculate_Type = 0;
											else if (value6.toString() == "Numerical")Caculate_Type = 1;//������1��ʾ
											else if (value6.toString() == "ASCII")Caculate_Type = 2;//ASC2��2��ʾ
											else if (value6.toString() == "HEX")Caculate_Type = 3;//HEX��3��ʾ
											else Caculate_Type = -1;
										}
										else
										{
											TableWrite("Caculate_Typeֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������Caculate_Type���ԣ�", 5);
										break;
									}
									if (object2.contains("StartBit"))//��ʼλ��ע��������bit������byte
									{
										QJsonValue value8 = object2.take("StartBit");
										if (value8.isString())
										{
											start_bit = value8.toString().toInt();
										}
										else
										{
											TableWrite("StartBitֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������StartBit���ԣ�", 5);
										break;
									}
									if (object2.contains("BitLength"))//λ���ȣ�ע����bit
									{
										QJsonValue value9 = object2.take("BitLength");
										if (value9.isString())
										{
											bit_length = value9.toString().toInt();
										}
										else
										{
											TableWrite("BitLengthֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������BitLength���ԣ�", 5);
										break;
									}
									
									if (object2.contains("ContentID"))//ֵ���ƣ�һ�����state״̬���ź�
									{
										QJsonValue value11 = object2.take("ContentID");
										if (value11.isString())
										{
											QMap<int, QString> temp_map = GetContent(file_content, value11.toString());
											if (temp_map.size() > 0)
											{
												temp_content_map = temp_map;
											}
										}
										else
										{
											TableWrite("ContentIDֵ�����ַ�����", 5);
											break;
										}
									}
									else
									{
										TableWrite("�ļ�������ContentID���ԣ�", 5);
										break;
									}
								
									if (Caculate_Type == 0)
									{
										int temp_cal = 0xFF;
										for (int i = 0; i < 8; i++)
										{
											if (!(i >= start_bit && i < start_bit + bit_length))
											{
												temp_cal = temp_cal - (1 << (7 - i));
											}
										}
										int temp_result = temp_cal & analyse_byte;
										if (temp_content_map.size() > 0)return temp_content_map[temp_result];
									}
									break;
								}
							}
							else
							{
								TableWrite("SendCommand_IDֵ�����ַ�����", 5);
								break;
							}
						}
						else
						{
							TableWrite("�ļ�������SendCommand_ID���ԣ�", 5);
							break;
						}
					}
				}
				else
				{
					TableWrite("ByteAnalyseֵ�������飡", 5);
					return "";
				}
			}
			else
			{
				TableWrite("�ļ�������ByteAnalyse���ԣ�", 5);
				return "";
			}
		}
		else
		{
			TableWrite("Json��ʽ����ȷ��", 5);
			return "";
		}
	}
	else
	{
		TableWrite("Json��ʽ����ȷ������λ�ã� " + QString::number(json_error.offset), 5);
		return "";
	}
	return "";
}





